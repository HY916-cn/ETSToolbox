#include<httplib.h>
#include<filesystem>
#include<cstdio>
#include<Windows.h>
#include"server.h"
#include"utils.h"
#include"logger.h"
#include<fstream>
#include<format>

namespace {
	bool launchAnswerConsole(const std::filesystem::path& scriptPath, DWORD& errorCode)
	{
		std::wstring systemDirectory(MAX_PATH, L'\0');
		const UINT systemDirectoryLength = GetSystemDirectoryW(
			systemDirectory.data(),
			static_cast<UINT>(systemDirectory.size())
		);
		if (systemDirectoryLength == 0 || systemDirectoryLength >= systemDirectory.size()) {
			errorCode = GetLastError();
			return false;
		}
		systemDirectory.resize(systemDirectoryLength);

		const auto powershellPath = std::filesystem::path(systemDirectory) /
			L"WindowsPowerShell" / L"v1.0" / L"powershell.exe";
		if (!std::filesystem::is_regular_file(powershellPath)) {
			errorCode = ERROR_FILE_NOT_FOUND;
			return false;
		}

		std::wstring commandLine = L"\"" + powershellPath.wstring() +
			L"\" -NoLogo -NoProfile -ExecutionPolicy Bypass -File \"" +
			scriptPath.wstring() + L"\"";
		STARTUPINFOW startupInfo{};
		startupInfo.cb = sizeof(startupInfo);
		PROCESS_INFORMATION processInfo{};
		const auto workingDirectory = scriptPath.parent_path().wstring();
		const BOOL created = CreateProcessW(
			powershellPath.c_str(),
			commandLine.data(),
			nullptr,
			nullptr,
			FALSE,
			CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
			nullptr,
			workingDirectory.c_str(),
			&startupInfo,
			&processInfo
		);
		if (!created) {
			errorCode = GetLastError();
			return false;
		}

		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
		errorCode = ERROR_SUCCESS;
		return true;
	}
}

ServerWrapper::ServerWrapper()
{
	const auto staticFilesPath = getStaticFilesDirectory();
	this->server.set_mount_point("/static", staticFilesPath.string());
	if (const char* appData = getenv("appdata"); appData != nullptr) {
		this->server.set_mount_point("/ets", (std::filesystem::path(appData) / "ETS").string());
	}
	this->server.Get("/api/read_config", [&](const httplib::Request& req, httplib::Response& res) {
		std::ifstream file(getStaticFilesDirectory() / "settings.json");
		if (file.is_open()) {
			std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			res.set_content(str,"application/json");
		}
		else {
			res.set_content("error","text/plain");
		}
		
	});
	this->server.Post("/api/write_config", [&](const httplib::Request& req, httplib::Response& res) {
		std::ofstream file(getStaticFilesDirectory() / "settings.json");
		if (file.is_open()) {
			file << req.body;
			file.close();
			res.set_content("ok", "text/plain; charset=utf-8");
		}
		else {
			res.set_content("error", "text/plain");
			file.close();
		}
	});
	this->server.Post("/api/open_answer_console", [&](const httplib::Request& req, httplib::Response& res) {
		const auto scriptPath = getStaticFilesDirectory() / "answer-console.ps1";
		if (!std::filesystem::is_regular_file(scriptPath)) {
			Logger::error("答案终端启动脚本不存在：{}", to_byte_string(scriptPath.wstring()));
			res.status = 404;
			res.set_content("答案终端启动脚本不存在", "text/plain; charset=utf-8");
			return;
		}

		DWORD errorCode = ERROR_SUCCESS;
		if (!launchAnswerConsole(scriptPath, errorCode)) {
			Logger::error("无法启动答案终端，Win32 错误码：{}", errorCode);
			res.status = 500;
			res.set_content(
				std::format("无法启动系统 PowerShell（Win32 错误码 {}）", errorCode),
				"text/plain; charset=utf-8"
			);
			return;
		}

		Logger::info("已打开答案终端");
		res.set_content("ok", "text/plain; charset=utf-8");
	});
}

void ServerWrapper::shutdown()
{
	if (this->server.is_running()) {
		this->server.stop();
	}
}

void ServerWrapper::listen()
{
	Logger::info("服务器开启于端口8080，前端目录：{}", to_byte_string(getStaticFilesDirectory().wstring()));
	std::thread t = std::thread([this]() {
		this->server.listen("127.0.0.1", 8080);
		});
	t.detach();
}

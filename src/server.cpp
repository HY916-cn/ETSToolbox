#include<httplib.h>
#include<filesystem>
#include<cstdio>
#include<Windows.h>
#include<Shellapi.h>
#include"server.h"
#include"utils.h"
#include"logger.h"
#include<fstream>

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
		const auto scriptPath = getStaticFilesDirectory() / "open-answer-console.cmd";
		if (!std::filesystem::is_regular_file(scriptPath)) {
			Logger::error("答案终端启动脚本不存在：{}", to_byte_string(scriptPath.wstring()));
			res.status = 404;
			res.set_content("answer console script not found", "text/plain; charset=utf-8");
			return;
		}

		const auto workingDirectory = scriptPath.parent_path().wstring();
		const auto result = ShellExecuteW(
			nullptr,
			L"open",
			scriptPath.c_str(),
			nullptr,
			workingDirectory.c_str(),
			SW_SHOWNORMAL
		);
		if (reinterpret_cast<INT_PTR>(result) <= 32) {
			Logger::error("无法打开答案终端，ShellExecuteW 错误码：{}", reinterpret_cast<INT_PTR>(result));
			res.status = 500;
			res.set_content("failed to open answer console", "text/plain; charset=utf-8");
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

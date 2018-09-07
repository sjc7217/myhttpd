#include "header.h"

Plugin::Plugin() {
	plugin_data = nullptr;
	plugin_is_loaded = false;
	setup_plugin = nullptr;
	remove_plugin = nullptr;
}

Plugin::~Plugin() {}

bool Plugin::Init(Connection *con, int index) {
	return true;
}

bool Plugin::RequestStart(Connection *con, int index) {
	return true;
}

bool Plugin::Read(Connection *con, int index) {
	return true;
}

bool Plugin::RequestEnd(Connection *con, int index) {
	return true;
}

bool Plugin::ResponseStart(Connection *con, int index) {
	return true;
}

plugin_state_t Plugin::Write(Connection *con, int index) {
	return PLUGIN_READY;
}

bool Plugin::ResponseEnd(Connection *con, int index) {
	return true;
}

void Plugin::Close(Connection *con, int index) {}

bool Plugin::Trigger(Worker *worker, int index) {
	return true;
}

bool Plugin::LoadPlugin(Worker *worker, int index) {
	return true;
}

void Plugin::FreePlugin(Worker *worker, int index) {}

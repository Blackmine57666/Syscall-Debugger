#include <debugwindow.h>
#include "configFile.h"

using namespace std;

Process* DebugWindow::getProcess(pid_t pid){
	for (Process *p : processes) {
		if (p->pid == pid) {
			return p;
		}
	}
	return nullptr;
}

void DebugWindow::killProcesses(){ // TODO hardcoder ca ?
	for(Process* p : processes){
		kill(p->pid, 9);
	}
}

// -----------------------------------

void DebugWindow::handleCallStart(Process& proc) {
	if(displayed->pid==proc.pid){
		dataMutex.lock();
		addEntryStart(*proc.currentCall);
		dataMutex.unlock();
	}

	if(proc.calls.size()==config::displayLimit){
		dataMutex.lock();
		delete proc.calls.front();
		proc.calls.pop_front();

		UI.callLogs->setRowCount(config::displayLimit);
		dataMutex.unlock();
	}else if(proc.calls.size()>config::displayLimit){
		throw runtime_error("Calls list too large !");
	}

}


void DebugWindow::handleCallReturn(Process& proc) {
	if (config::doChilds && proc.currentCall->entry.id == 56) { // TODO 56 doit pas être hardcodé
		Process* newChild = getProcess(proc.currentCall->exit.rval);
		if(newChild==nullptr){
			newChild = handleChildCreate(proc.currentCall->exit.rval);
		}

		newChild->treeItem = new QTreeWidgetItem;
		newChild->treeItem->setText(0, QString(to_string(newChild->pid).c_str()));
		proc.treeItem->addChild(newChild->treeItem);
	}

	if(displayed->pid==proc.pid){
		dataMutex.lock();
		addEntryEnd(*proc.currentCall);
		dataMutex.unlock();
	}
}

Process* DebugWindow::handleChildCreate(pid_t pid){ // Warning : Still need to apply Tree Item Widget. besoin = get parent parent from here
	auto* newChild = new Process();
	newChild->pid = pid;
	processes.insert(newChild);

	return newChild;
}

bool DebugWindow::handleChildExit(pid_t stopped){ // true is there is no more processes

	Process* proc = nullptr;
	bool empty = true;
	for (Process *p : processes) {
		if (p->running){
			if(p->pid == stopped) {
				p->running = false;
				proc = p;
			}else{
				empty = false;
			}
			if(!empty&&proc!=nullptr)break; // TODO jsp si vraiment opti
		}
	}
	if(proc==nullptr){
		cerr << "Invalid child exited : " << stopped << endl;
	}else{
		return empty;
	}
	return false;
}
#include "Headers/Debugger.hpp"
#include "Headers/Emulator.hpp"

/*
 *  Binds the debugger to a new emulator object
 */
void Debugger::bind(Emulator* newEmulator){
    emulator = newEmulator;
}

/*
 *  Logger functions
 */
void Debugger::emuLog(std::string message, LOGLEVEL lvl){
    if(emulator->getConfig()->noLogging()) return;
    std::cout << (lvl == 10 ? "[@] " : \
					   lvl == 5 ? "[!] " : "[i] " ) << message << "\n";
}

void Debugger::emuLog(std::string message){
    if(emulator->getConfig()->noLogging()) return;
    std::cout << "[i] " << message << "\n";
    if(emulator->getConfig()->graphicsDisabled()) std::cout.flush();
}



///
///  Breakpoints
///



void Debugger::handleAddressBreakpoint(uint16_t address) {
    if(this->checkAddressBreakpoint(address)){
        emulator->triggerBreak("Reached breakpoint at 0x" + Utils::decHex(address));
    }
}

void Debugger::handleInstructionBreakpoint(InstructionBreakpoint op) {
    if(this->checkInstructionBreakpoint(op)){
        emulator->triggerBreak("Instruction breakpoint $" + Utils::decHex(op.op,2));
    }
}

void Debugger::handleMemoryBreakpoint(MemoryBreakpoint operation) {
    if(this->checkMemoryBreakpoint(operation)){
        emulator->triggerBreak("Memory breakpoint");
    }
}

void Debugger::addAddressBreakpoint(uint16_t address) {
    addressBreakpoints.push_back(address);
}

void Debugger::addOpBreakpoint(uint8_t op, bool isCB) {
    instructionBreakpoints.push_back(InstructionBreakpoint(op, isCB));
}

void Debugger::addMemoryBreakpoint(MemoryBreakpoint breakpoint) {
    memoryBreakpoints.push_back(breakpoint);
}

bool Debugger::checkAddressBreakpoint(uint16_t address) {
    for(uint16_t addr : addressBreakpoints){
        if(addr == address) return true;
    }
    return false;
}

bool Debugger::checkInstructionBreakpoint(InstructionBreakpoint op) {
    for(InstructionBreakpoint breakpoint : instructionBreakpoints){
        if(op.op == breakpoint.op){
            if(op.cb == breakpoint.cb) return true;
        }
    }
    return false;
}

//  TODO: fix
bool Debugger::checkMemoryBreakpoint(MemoryBreakpoint operation) {
    for(MemoryBreakpoint breaks : memoryBreakpoints){
        if(operation.addr == breaks.addr){
            bool ret = false;
            if(operation.r && breaks.r){
                if(operation.v && breaks.v) {
                    ret |= (operation.val == breaks.val);
                } else {
                    ret = true;
                }
            }
            if (operation.w && breaks.w){
                if(operation.v && breaks.v){
                    ret |= (operation.val == breaks.val);
                } else {
                    ret = true;
                }
            }
            return ret;
        }
    }
    return false;
}


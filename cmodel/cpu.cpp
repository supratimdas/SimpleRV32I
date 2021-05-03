#include <iostream>
#include "SimpleRV32I.h"
#include "SimpleRV32I_utils.h"


int main() {
    SimpleRV32I cpuModel = SimpleRV32I();
    cpuModel.loadProgram();
    cpuModel.loadData();
    while(!cpuModel.step()); //step through the program until the model indicates execution complete
    cpuModel.dumpData();
    cpuModel.dumpRegs();
    return 0;
}

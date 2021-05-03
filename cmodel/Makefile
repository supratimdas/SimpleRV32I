CC=g++


rv32i_sim : SimpleRV32I.cpp  SimpleRV32I.h  SimpleRV32I_utils.cpp  SimpleRV32I_utils.h cpu.cpp
	${CC} -o rv32i_sim cpu.cpp SimpleRV32I.cpp SimpleRV32I_utils.cpp


.PHONY clean:
	rm -Rf *.o *.out *.txt rv32i_sim

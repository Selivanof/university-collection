module datapath 
#(parameter INITIAL_PC = 32'h00400000)
(
    input clk,                      // Clock
    input rst,                      // Sync Reset
    input [31:0] instr,             // Instruction Data from Instruction Mem
    input PCSrc,                    // PC Source
    input ALUSrc,                   // ALU's second operator Source
    input RegWrite,                 // Write to registers
    input MemToReg,                 // Register Input MUX
    input [3:0] ALUCtrl,            // Operation for ALU
    input loadPC,                   // Update PC with a new value
    output reg [31:0] PC,           // Program Counter
    output Zero,                    // ALU zero output
    output [31:0] dAddress,         // Data Memory Address
    output [31:0] dWriteData,       // Data to be written to Memory
    input [31:0] dReadData,         // Data to be read from Memory
    output [31:0] WriteBackData     // WriteBack data that return to the registers
);

// ================== Internal Signal Declarations ==================

// RISC-V Instructions
wire[31:0] imm;
wire[6:0] opcode = instr[6:0];

// Registers
wire[31:0] readData1,readData2,writeData;
wire[4:0] readReg1,readReg2,writeReg;

// ALU
wire[31:0] op1, op2, alu_result;


// ======= Immediate Value Extraction for RISC-V Instructions =======

// B-TYPE
assign imm = (opcode == 7'b1100011) ? {instr[31], instr[7], instr[30:25], instr[11:8], 1'b0} :
// I-TYPE
             (opcode == 7'b0000011) ? { {20{instr[31]}}, instr[31:20] }:
             (opcode == 7'b0010011) ? { {20{instr[31]}}, instr[31:20] }:
// S-TYPE
             (opcode == 7'b0100011) ? {instr[31:25], instr[11:7]} : 32'b0;


// =========================== Registers ============================
regfile myRegFile (
    .clk(clk),
    .readReg1(readReg1),
    .readReg2(readReg2),
    .writeReg(writeReg),
    .writeData(writeData),
    .write(RegWrite), 
    .readData1(readData1),
    .readData2(readData2)
);
assign readReg1 = instr[19:15];
assign readReg2 = instr[24:20];
assign writeReg = instr[11:7];

assign writeData = (MemToReg == 0) ? alu_result : dReadData;
assign WriteBackData = writeData;


// ============================== ALU ===============================

alu uut (
   .op1(op1),
   .op2(op2),
   .alu_op(ALUCtrl),
   .zero(Zero),
   .result(alu_result)
);
assign op1 = readData1;
assign op2 = (ALUSrc == 0) ? readData2 : imm;


// ========================== DATA MEMORY ===========================

assign dAddress = alu_result;
assign dWriteData = readData2;


// ========================= CLOCK POSEDGE ==========================

always@(posedge clk)
begin

// RESET
if (rst)

    PC <= INITIAL_PC;
else
    // Update PC
    if (loadPC)
        if (PCSrc)
            PC <= PC + imm;
        else 
            PC <= PC + 4;
end

endmodule

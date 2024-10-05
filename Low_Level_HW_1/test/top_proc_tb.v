`timescale 1ns/1ps

module multicycle_tb;

// =========================== Parameters ===========================
localparam INITIAL_PC = 32'h00400000;

// ======================= Testbench Signals ========================
reg clk;
reg rst;
wire [31:0] instr;
wire [31:0] dReadData;
wire [31:0] PC;
wire [31:0] dAddress;
wire [31:0] dWriteData;
wire MemRead;
wire MemWrite;
wire [31:0] WriteBackData;

// ============================== UUT ==============================
multicycle #(INITIAL_PC) uut (
    .clk(clk),
    .rst(rst),
    .instr(instr),
    .dReadData(dReadData),
    .PC(PC),
    .dAddress(dAddress),
    .dWriteData(dWriteData),
    .MemRead(MemRead),
    .MemWrite(MemWrite),
    .WriteBackData(WriteBackData)
);

// =========================== Memories ============================
INSTRUCTION_MEMORY proc_instruction_memory (
    .clk(clk),
    .addr(PC[8:0]),
    .dout(instr)
);

DATA_MEMORY data_memory_inst (
    .clk(clk),
    .we(MemWrite),
    .addr(dAddress[8:0]),
    .din(dWriteData),
    .dout(dReadData)
);

// ============================= Clock ==============================
initial begin
    clk = 0;
    forever #5 clk = ~clk;
end

// ============================= Reset =============================
initial begin
    // Apply reset
    rst = 1;
    #10 rst = 0;
    // End simulation after a certain period
    #2000 $finish;
end

endmodule
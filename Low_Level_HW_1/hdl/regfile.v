module regfile (
  input clk,
  input [4:0] readReg1,         // Address for read port 1
  input [4:0] readReg2,         // Address for read port 2
  input [4:0] writeReg,         // Address for write port
  input [31:0] writeData,       // Data to be written
  input write,                  // Write control signal
  output reg [31:0] readData1,  // Data to be read from port 1
  output reg [31:0] readData2   // Data to be read from port 2
);

reg [31:0] registers [0:31];

// Initialization block to set all registers to zero
integer i; 
initial begin
  for (i = 0; i < 32; i = i + 1) begin
    registers[i] = 32'b0;
  end
end

always @(posedge clk) 
begin
  // READ
  readData1 <= registers[readReg1];
  readData2 <= registers[readReg2];
  
  // WRITE
  if (write) begin
      registers[writeReg] <= writeData;

  end
end
endmodule
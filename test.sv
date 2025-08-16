module TestModule (
    input logic clk,
    input logic rstn,

    input  logic A,
    output logic B

);

    assing B = ~A;

endmodule

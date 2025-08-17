module SubModule #(
    parameter logic ENABLE = 1
) (
    input  logic A,
    input  logic B,
    output logic C
);
    assign C = ~A & B;
endmodule

module TestModule (
    input logic clk,
    input logic rstn,

    input  logic A,
    output logic O
);
    SubModule #(
        .ENABLE(0)
    ) module_inst (
        .A(A),
        .B(rstn),
        .O(O)
    );
endmodule

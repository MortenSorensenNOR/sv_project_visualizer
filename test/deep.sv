// This is a comment
module SubSubSubModule (
    input  logic A,
    output logic B
);
    assign B = ~A;
endmodule

module SubSubModule #(
    parameter logic BEST_NUMBER = 42,
    parameter string MESSAGE = "HELLO WORLD!"
) (
    input  logic A,
    output logic B
);
    SubSubSubModule sub_inst (
        .A(A),
        .B(B)
    );
endmodule

module SubModule #(
    parameter logic ENABLE = 1,
    parameter string TEST = "HELLO WORLD!"
) (
    input  logic A,
    input  logic B,
    output logic C
);

    logic D;
    SubSubModule module_inst_1 (
        .A(A),
        .B(C)
    );

    logic E;
    SubSubModule module_inst_2 (
        .A(B),
        .B(E)
    );

    assign C = D | E;
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

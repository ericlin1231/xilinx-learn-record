`timescale 1ns / 1ps

module tmds_serializer (
    input        pix_clk,
    input        serial_clk,
    input        rst,
    input  [9:0] data_in,
    output       data_out
);

    wire cascade_di, cascade_do;
    wire cascade_ti, cascade_to;

    OSERDESE2 #(
        .DATA_RATE_OQ   ("DDR"),
        .DATA_RATE_TQ   ("SDR"),
        .DATA_WIDTH     (10),
        .SERDES_MODE    ("MASTER"),
        .TRISTATE_WIDTH (1)
    ) oserdes_m (
        .OQ             (data_out),
        .CLK            (serial_clk),
        .CLKDIV         (pix_clk),
        .D1             (data_in[0]),
        .D2             (data_in[1]),
        .D3             (data_in[2]),
        .D4             (data_in[3]),
        .D5             (data_in[4]),
        .D6             (data_in[5]),
        .D7             (data_in[6]),
        .D8             (data_in[7]),
        .TCE            (1'b0),
        .OCE            (1'b1),
        .TBYTEIN        (1'b0),
        .RST            (rst),
        .SHIFTIN1       (cascade_do),
        .SHIFTIN2       (cascade_to),
        .SHIFTOUT1      (),
        .SHIFTOUT2      (),
        .T1             (1'b0),
        .T2             (1'b0),
        .T3             (1'b0),
        .T4             (1'b0),
        .TBYTEOUT       (),
        .OFB            (),
        .TFB            ()
    );

    OSERDESE2 #(
        .DATA_RATE_OQ   ("DDR"),
        .DATA_RATE_TQ   ("SDR"),
        .DATA_WIDTH     (10),
        .SERDES_MODE    ("SLAVE"),
        .TRISTATE_WIDTH (1)
    ) oserdes_s (
        .OQ             (),
        .CLK            (serial_clk),
        .CLKDIV         (pix_clk),
        .D1             (1'b0),
        .D2             (1'b0),
        .D3             (data_in[8]),
        .D4             (data_in[9]),
        .D5             (1'b0),
        .D6             (1'b0),
        .D7             (1'b0),
        .D8             (1'b0),
        .TCE            (1'b0),
        .OCE            (1'b1),
        .TBYTEIN        (1'b0),
        .RST            (rst),
        .SHIFTIN1       (1'b0),
        .SHIFTIN2       (1'b0),
        .SHIFTOUT1      (cascade_do),
        .SHIFTOUT2      (cascade_to),
        .T1             (1'b0),
        .T2             (1'b0),
        .T3             (1'b0),
        .T4             (1'b0),
        .TBYTEOUT       (),
        .OFB            (),
        .TFB            ()
    );

endmodule
`timescale 1ns / 1ps

module hdmi_out (
    input         PXLCLK_I,
    input         PXLCLK_5X_I,
    input         LOCKED_I,
    input         RST_N,

    input         VGA_HS,
    input         VGA_VS,
    input         VGA_DE,
    input  [23:0] VGA_RGB,

    output        HDMI_CLK_P,
    output        HDMI_CLK_N,
    output        HDMI_D2_P,
    output        HDMI_D2_N,
    output        HDMI_D1_P,
    output        HDMI_D1_N,
    output        HDMI_D0_P,
    output        HDMI_D0_N
);

    wire reset = ~RST_N | ~LOCKED_I;

    wire [7:0] red   = VGA_RGB[23:16];
    wire [7:0] green = VGA_RGB[15:8];
    wire [7:0] blue  = VGA_RGB[7:0];

    wire [9:0] tmds_r, tmds_g, tmds_b;

    tmds_encoder enc_r (
        .clk   (PXLCLK_I),
        .data  (red),
        .c     (2'b00),
        .de    (VGA_DE),
        .q_out (tmds_r)
    );

    tmds_encoder enc_g (
        .clk   (PXLCLK_I),
        .data  (green),
        .c     (2'b00),
        .de    (VGA_DE),
        .q_out (tmds_g)
    );

    tmds_encoder enc_b (
        .clk   (PXLCLK_I),
        .data  (blue),
        .c     ({VGA_VS, VGA_HS}),
        .de    (VGA_DE),
        .q_out (tmds_b)
    );

    wire ser_r, ser_g, ser_b;

    tmds_serializer ser_r_inst (
        .pix_clk    (PXLCLK_I),
        .serial_clk (PXLCLK_5X_I),
        .rst        (reset),
        .data_in    (tmds_r),
        .data_out   (ser_r)
    );

    tmds_serializer ser_g_inst (
        .pix_clk    (PXLCLK_I),
        .serial_clk (PXLCLK_5X_I),
        .rst        (reset),
        .data_in    (tmds_g),
        .data_out   (ser_g)
    );

    tmds_serializer ser_b_inst (
        .pix_clk    (PXLCLK_I),
        .serial_clk (PXLCLK_5X_I),
        .rst        (reset),
        .data_in    (tmds_b),
        .data_out   (ser_b)
    );

    OBUFDS #(.IOSTANDARD("TMDS_33")) obufds_d2 (
        .I (ser_r), .O (HDMI_D2_P), .OB (HDMI_D2_N)
    );
    OBUFDS #(.IOSTANDARD("TMDS_33")) obufds_d1 (
        .I (ser_g), .O (HDMI_D1_P), .OB (HDMI_D1_N)
    );
    OBUFDS #(.IOSTANDARD("TMDS_33")) obufds_d0 (
        .I (ser_b), .O (HDMI_D0_P), .OB (HDMI_D0_N)
    );

    OBUFDS #(.IOSTANDARD("TMDS_33")) obufds_clk (
        .I (PXLCLK_I), .O (HDMI_CLK_P), .OB (HDMI_CLK_N)
    );

endmodule
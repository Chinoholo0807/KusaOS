#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
PUBLIC void hello_kusa()
{
    int i;
    disp_pos = 0;
    for (i = 0; i < 80 * 4; i++)
    {
        disp_str(" ");
    }
    disp_color_str("                                                                                ", 0x8);
    disp_color_str("                                                                                ", 0x8);
    disp_color_str("                                                                                ", 0x8);
    disp_color_str("                                                             2M       Mr        ", 0x8);
    disp_color_str("                                                              MWX    2M7        ", 0x8);
    disp_color_str("                                                              iZZXXaa028        ", 0x8);
    disp_color_str("                                                              iZZ227XXa2a       ", 0x8);
    disp_color_str("                                                              XZ;r;rrrX7X2      ", 0x8);
    disp_color_str("          /$$   /$$ /$$   /$$  /$$$$$$   /$$$$$$              :7;:i;ri7rrX:     ", 0x8);
    disp_color_str("         | $$  /$$/| $$  | $$ /$$__  $$ /$$__  $$             :7ii,:;i;7;X;     ", 0x8);
    disp_color_str("         | $$ /$$/ | $$  | $$| $$  \\__/| $$  \\ $$             ,X :r,ii;7;7r     ", 0x8);
    disp_color_str("         | $$$$$/  | $$  | $$|  $$$$$$ | $$$$$$$$              2B ,MZ,:X;7r     ", 0x8);
    disp_color_str("         | $$  $$  | $$  | $$ \\____  $$| $$__  $$              ;7  ii,iZ7r2     ", 0x8);
    disp_color_str("         | $$\\  $$ | $$  | $$ /$$  \\ $$| $$  | $$              7S   X;rB2r27    ", 0x8);
    disp_color_str("         | $$ \\  $$|  $$$$$$/|  $$$$$$/| $$  | $$              SM8X,27SZZaXZ    ", 0x8);
    disp_color_str("         |__/  \\__/ \\______/  \\______/ |__/  |__/             ;M@WM2:aZBZ8SB:   ", 0x8);
    disp_color_str("                                                              ;MrZMMMMWM8aMM    ", 0x8);
    disp_color_str("                                                                  :  : ;M8:,    ", 0x8);
    disp_color_str("                                                            M    i     ,M@      ", 0x8);
    disp_color_str("                                                            ;    00B00Z70Mi     ", 0x8);
    disp_color_str("                                                                  ::,   ;       ", 0x8);
    disp_color_str("                                                                  S0            ", 0x8);
    disp_color_str("                                                                                ", 0x8);
    disp_color_str("                                                                                ", 0x8);
    disp_color_str("                                                                                ", 0x8);

    disp_pos = 0;
    for (i = 0; i < 80 * 4; i++)
    {
        disp_str(" ");
    }
    milli_delay(10000);

    disp_color_str("                                                                                ", 0x1);
    disp_color_str("                                                                                ", 0x1);
    disp_color_str("                                                                X    r          ", 0x1);
    disp_color_str("                                                               SM    Mr         ", 0x1);
    disp_color_str("                                                               W0   ,B0         ", 0x1);
    disp_color_str("                                                               XaSX2aZZZ,       ", 0x1);
    disp_color_str("                                                              ;8ZS2X7722Z,      ", 0x1);
    disp_color_str("                                                              ,Z;rrrrrX7X2      ", 0x1);
    disp_color_str("          /$$   /$$ /$$   /$$  /$$$$$$   /$$$$$$              ,X;:ii7i77rX;     ", 0x1);
    disp_color_str("         | $$  /$$/| $$  | $$ /$$__  $$ /$$__  $$             :X:i,,;i;7;7r     ", 0x1);
    disp_color_str("         | $$ /$$/ | $$  | $$| $$  \\__/| $$  \\ $$              X ,r,::;r;77     ", 0x1);
    disp_color_str("         | $$$$$/  | $$  | $$|  $$$$$$ | $$$$$$$$              20  M8 :7;7r     ", 0x1);
    disp_color_str("         | $$  $$  | $$  | $$ \\____  $$| $$__  $$              ;7  i;,iZrr2     ", 0x1);
    disp_color_str("         | $$\\  $$ | $$  | $$ /$$  \\ $$| $$  | $$              72   XirB2rS7    ", 0x1);
    disp_color_str("         | $$ \\  $$|  $$$$$$/|  $$$$$$/| $$  | $$              SM8X,2r28ZaXZ    ", 0x1);
    disp_color_str("         |__/  \\__/ \\______/  \\______/ |__/  |__/             iMW@MZ;a8W8820r   ", 0x1);
    disp_color_str("                                                              :M;XMMMM@M0aWM,   ", 0x1);
    disp_color_str("                                                                  ,  , ;ZS2     ", 0x1);
    disp_color_str("                                                            M    7     :M@W     ", 0x1);
    disp_color_str("                                                            ;    B8B0BZSa;MM    ", 0x1);
    disp_color_str("                                                                  ,,  ,         ", 0x1);
    disp_color_str("                                                                  BX            ", 0x1);
    disp_color_str("                                                                                ", 0x1);
    disp_color_str("                                                                                ", 0x1);
    disp_color_str("                                                                                ", 0x1);

    disp_pos = 0;
    for (i = 0; i < 80 * 4; i++)
    {
        disp_str(" ");
    }
    milli_delay(10000);

    disp_color_str("                                                                                ", 0x6);
    disp_color_str("                                                                                ", 0x6);
    disp_color_str("                                                                                ", 0x6);
    disp_color_str("                                                             8B      ,M         ", 0x6);
    disp_color_str("                                                              M@Xi   ZM,        ", 0x6);
    disp_color_str("                                                              ,0XZ7aaB28:       ", 0x6);
    disp_color_str("                                                               ZaS2XXX228i      ", 0x6);
    disp_color_str("                                                              ,Z;;;;rr777a      ", 0x6);
    disp_color_str("          /$$   /$$ /$$   /$$  /$$$$$$   /$$$$$$               X;::ir;rr7Xr     ", 0x6);
    disp_color_str("         | $$  /$$/| $$  | $$ /$$__  $$ /$$__  $$              X:i:,i;iX;77     ", 0x6);
    disp_color_str("         | $$ /$$/ | $$  | $$| $$  \\__/| $$  \\ $$              X,,riiii7rr7     ", 0x6);
    disp_color_str("         | $$$$$/  | $$  | $$|  $$$$$$ | $$$$$$$$              XW, MB,:X;77     ", 0x6);
    disp_color_str("         | $$  $$  | $$  | $$ \\____  $$| $$__  $$              r;  :i,:a7rS     ", 0x6);
    disp_color_str("         | $$\\  $$ | $$  | $$ /$$  \\ $$| $$  | $$              Xa   7;;B2;SX    ", 0x6);
    disp_color_str("         | $$ \\  $$|  $$$$$$/|  $$$$$$/| $$  | $$              2@8Xi27S8Z2XZ,   ", 0x6);
    disp_color_str("         |__/  \\__/ \\______/  \\______/ |__/  |__/             iMW@M2;a8WZ0aZX   ", 0x6);
    disp_color_str("                                                              :W;XMMMMWMZ2WM;   ", 0x6);
    disp_color_str("                                                                     : X@Zrr    ", 0x6);
    disp_color_str("                                                            M    i     ,MM      ", 0x6);
    disp_color_str("                                                            ;    80B0BZ2SW:     ", 0x6);
    disp_color_str("                                                                  ,,, ,         ", 0x6);
    disp_color_str("                                                                  M;    ,       ", 0x6);
    disp_color_str("                                                                  ,             ", 0x6);
    disp_color_str("                                                                                ", 0x6);
    disp_color_str("                                                                                ", 0x6);

    disp_pos = 0;
    for (i = 0; i < 80 * 4; i++)
    {
        disp_str(" ");
    }
    milli_delay(10000);

    disp_color_str("                                                                                ", 0x5);
    disp_color_str("                                                                                ", 0x5);
    disp_color_str("                                                                                ", 0x5);
    disp_color_str("                                                             2M       Mr        ", 0x5);
    disp_color_str("                                                              MBX    aM7        ", 0x5);
    disp_color_str("                                                              iaaXXaZ028        ", 0x5);
    disp_color_str("                                                              iZZ2S7XXa2a       ", 0x5);
    disp_color_str("                                                              XZ;r;rrrX7X2      ", 0x5);
    disp_color_str("          /$$   /$$ /$$   /$$  /$$$$$$   /$$$$$$              :7;:i;ri7rrX:     ", 0x5);
    disp_color_str("         | $$  /$$/| $$  | $$ /$$__  $$ /$$__  $$             :7:i,:;i;7;X;     ", 0x5);
    disp_color_str("         | $$ /$$/ | $$  | $$| $$  \\__/| $$  \\ $$             ,7 :r,ii;7;7r     ", 0x5);
    disp_color_str("         | $$$$$/  | $$  | $$|  $$$$$$ | $$$$$$$$              20 ,MZ,:Xi7r     ", 0x5);
    disp_color_str("         | $$  $$  | $$  | $$ \\____  $$| $$__  $$              ;7  ii,ia7r2     ", 0x5);
    disp_color_str("         | $$\\  $$ | $$  | $$ /$$  \\ $$| $$  | $$              7S   X;rB2r2r    ", 0x5);
    disp_color_str("         | $$ \\  $$|  $$$$$$/|  $$$$$$/| $$  | $$              XM8X,2XSZZaXZ    ", 0x5);
    disp_color_str("         |__/  \\__/ \\______/  \\______/ |__/  |__/             ;MWWM2:ZZBZ8SB:   ", 0x5);
    disp_color_str("                                                              ;MrZMMMMWM02MM    ", 0x5);
    disp_color_str("                                                                  :  : ;M8:,    ", 0x5);
    disp_color_str("                                                            M    i     ,M@      ", 0x5);
    disp_color_str("                                                            ;    80000Z70Mi     ", 0x5);
    disp_color_str("                                                                  ::, , ;       ", 0x5);
    disp_color_str("                                                                  S0   ,        ", 0x5);
    disp_color_str("                                                                                ", 0x5);
    disp_color_str("                                                                                ", 0x5);
    disp_color_str("                                                                                ", 0x5);

    disp_pos = 0;
    for (i = 0; i < 80 * 4; i++)
    {
        disp_str(" ");
    }
    milli_delay(10000);

    disp_color_str("                                                                                ", 0x4);
    disp_color_str("                                                                         ,      ", 0x4);
    disp_color_str("                                                                       ;,       ", 0x4);
    disp_color_str("                                                             7Z       :Mi       ", 0x4);
    disp_color_str("                                                              MBi;    0Wr       ", 0x4);
    disp_color_str("                                                              iMXZiXXZ8Z7       ", 0x4);
    disp_color_str("                                                               Za2aSXXXaa,      ", 0x4);
    disp_color_str("                                                              rZ7;i;;r;XXa      ", 0x4);
    disp_color_str("          /$$   /$$ /$$   /$$  /$$$$$$   /$$$$$$              ,2;i:,;rir7Xr     ", 0x4);
    disp_color_str("         | $$  /$$/| $$  | $$ /$$__  $$ /$$__  $$              X,ii:,;irrrX     ", 0x4);
    disp_color_str("         | $$ /$$/ | $$  | $$| $$  \\__/| $$  \\ $$              S  ;r7i:;7rX     ", 0x4);
    disp_color_str("         | $$$$$/  | $$  | $$|  $$$$$$ | $$$$$$$$              7Br ZM7 ;r7;     ", 0x4);
    disp_color_str("         | $$  $$  | $$  | $$ \\____  $$| $$__  $$              ;7   ,:,7SX;     ", 0x4);
    disp_color_str("         | $$\\  $$ | $$  | $$ /$$  \\ $$| $$  | $$              SS   ,2iZaXX     ", 0x4);
    disp_color_str("         | $$ \\  $$|  $$$$$$/|  $$$$$$/| $$  | $$              B@ZS:,SX802a     ", 0x4);
    disp_color_str("         |__/  \\__/ \\______/  \\______/ |__/  |__/             @Z0BWXr8882B2Z    ", 0x4);
    disp_color_str("                                                              M8B;ZMMMM7 00M,   ", 0x4);
    disp_color_str("                                                               rS:X :;  ,M@M    ", 0x4);
    disp_color_str("                                                            M    :,    :8M      ", 0x4);
    disp_color_str("                                                            r    ;WB88aaB@r     ", 0x4);
    disp_color_str("                                                                  i;r   :MM7    ", 0x4);
    disp_color_str("                                                                   , ,r         ", 0x4);
    disp_color_str("                                                                      8         ", 0x4);
    disp_color_str("                                                                                ", 0x4);
    disp_color_str("                                                                                ", 0x4);

    disp_pos = 0;
    for (i = 0; i < 80 * 4; i++)
    {
        disp_str(" ");
    }
    milli_delay(10000);

    disp_color_str("                                                                                ", 0x3);
    disp_color_str("                                                                                ", 0x3);
    disp_color_str("                                                                        X       ", 0x3);
    disp_color_str("                                                              B        WM       ", 0x3);
    disp_color_str("                                                              8M;7    2WZ       ", 0x3);
    disp_color_str("                                                               @XZ;XX2ZZS       ", 0x3);
    disp_color_str("                                                               2Z22SXXSaZ:      ", 0x3);
    disp_color_str("                                                              ;87;;;;rrX7a      ", 0x3);
    disp_color_str("          /$$   /$$ /$$   /$$  /$$$$$$   /$$$$$$              :S;i:,;rir7X7     ", 0x3);
    disp_color_str("         | $$  /$$/| $$  | $$ /$$__  $$ /$$__  $$             ,7,ii:,;irrrX     ", 0x3);
    disp_color_str("         | $$ /$$/ | $$  | $$| $$  \\__/| $$  \\ $$             ,X  ;rri:rr;X     ", 0x3);
    disp_color_str("         | $$$$$/  | $$  | $$|  $$$$$$ | $$$$$$$$              70r ZM; rrrr     ", 0x3);
    disp_color_str("         | $$  $$  | $$  | $$ \\____  $$| $$__  $$              ;7    ::7SX7     ", 0x3);
    disp_color_str("         | $$\\  $$ | $$  | $$ /$$  \\ $$| $$  | $$              7S    2:2a7S     ", 0x3);
    disp_color_str("         | $$ \\  $$|  $$$$$$/|  $$$$$$/| $$  | $$              ZWZS:,2XZ8Sai    ", 0x3);
    disp_color_str("         |__/  \\__/ \\______/  \\______/ |__/  |__/             BB0ZWSr80Ba8Sa    ", 0x3);
    disp_color_str("                                                              @MW8MMMMa;WZ0W    ", 0x3);
    disp_color_str("                                                               :82i  r :Ma0B    ", 0x3);
    disp_color_str("                                                            M    :    : 0MWZ    ", 0x3);
    disp_color_str("                                                            r    iB8882ar ZM8   ", 0x3);
    disp_color_str("                                                                  i;SZ          ", 0x3);
    disp_color_str("                                                                 ,   ,2         ", 0x3);
    disp_color_str("                                                                       S        ", 0x3);
    disp_color_str("                                                                                ", 0x3);
    disp_color_str("                                                                                ", 0x3);

    disp_pos = 0;
    for (i = 0; i < 80 * 4; i++)
    {
        disp_str(" ");
    }
    milli_delay(10000);

    disp_color_str("                                                                                ", 0x2);
    disp_color_str("                                                                                ", 0x2);
    disp_color_str("                                                                                ", 0x2);
    disp_color_str("                                                              B       i         ", 0x2);
    disp_color_str("                                                              ZMSi:   Mi        ", 0x2);
    disp_color_str("                                                               80X2778B8i       ", 0x2);
    disp_color_str("                                                               iZX2SXX2ZBX      ", 0x2);
    disp_color_str("                                                               XS;ir;r;X7Z;     ", 0x2);
    disp_color_str("          /$$   /$$ /$$   /$$  /$$$$$$   /$$$$$$               i7i:,;r;r772     ", 0x2);
    disp_color_str("         | $$  /$$/| $$  | $$ /$$__  $$ /$$__  $$              riii::rirrrS,    ", 0x2);
    disp_color_str("         | $$ /$$/ | $$  | $$| $$  \\__/| $$  \\ $$              7; i;iii;7;S     ", 0x2);
    disp_color_str("         | $$$$$/  | $$  | $$|  $$$$$$ | $$$$$$$$              iWX ZM; rrrX     ", 0x2);
    disp_color_str("         | $$  $$  | $$  | $$ \\____  $$| $$__  $$              i7   ;:,XXrX     ", 0x2);
    disp_color_str("         | $$\\  $$ | $$  | $$ /$$  \\ $$| $$  | $$              X8   7ri0SrS     ", 0x2);
    disp_color_str("         | $$ \\  $$|  $$$$$$/|  $$$$$$/| $$  | $$              Z822 X72B87Xa,   ", 0x2);
    disp_color_str("         |__/  \\__/ \\______/  \\______/ |__/  |__/             ;Ma0M2aZ0iZ0a2a   ", 0x2);
    disp_color_str("                                                              ,B22MMWZ0SBa8BB   ", 0x2);
    disp_color_str("                                                                  i :: 0MS0@i   ", 0x2);
    disp_color_str("                                                            M    i      MM2     ", 0x2);
    disp_color_str("                                                            ;    808Z8ZZ7WZ     ", 0x2);
    disp_color_str("                                                                  :,:ii         ", 0x2);
    disp_color_str("                                                                 , ,;           ", 0x2);
    disp_color_str("                                                                    8           ", 0x2);
    disp_color_str("                                                                                ", 0x2);
    disp_color_str("                                                                                ", 0x2);

    disp_pos = 0;
    for (i = 0; i < 80 * 4; i++)
    {
        disp_str(" ");
    }
    milli_delay(10000);

    disp_color_str("                                                                                ", 0x7);
    disp_color_str("                                                                                ", 0x7);
    disp_color_str("                                                                        7       ", 0x7);
    disp_color_str("                                                              W        MW       ", 0x7);
    disp_color_str("                                                              BM;r    2@2       ", 0x7);
    disp_color_str("                                                               @XaiXX2ZZX       ", 0x7);
    disp_color_str("                                                               2Za2SXXSaZ:      ", 0x7);
    disp_color_str("                                                              i07;i;;r;X7a      ", 0x7);
    disp_color_str("          /$$   /$$ /$$   /$$  /$$$$$$   /$$$$$$               Sii::;rirrXX     ", 0x7);
    disp_color_str("         | $$  /$$/| $$  | $$ /$$__  $$ /$$__  $$              X:ii:,;irrrX     ", 0x7);
    disp_color_str("         | $$ /$$/ | $$  | $$| $$  \\__/| $$  \\ $$              S, ir7i:;7;X     ", 0x7);
    disp_color_str("         | $$$$$/  | $$  | $$|  $$$$$$ | $$$$$$$$              707 ZMr r77r     ", 0x7);
    disp_color_str("         | $$  $$  | $$  | $$ \\____  $$| $$__  $$              iX    :,7SX7     ", 0x7);
    disp_color_str("         | $$\\  $$ | $$  | $$ /$$  \\ $$| $$  | $$              X2    2:aaXX     ", 0x7);
    disp_color_str("         | $$ \\  $$|  $$$$$$/|  $$$$$$/| $$  | $$              8@ZS:,2X802a,    ", 0x7);
    disp_color_str("         |__/  \\__/ \\______/  \\______/ |__/  |__/             @08Z@Sr80B20Sa    ", 0x7);
    disp_color_str("                                                              M@W8MMMM0,8a8@    ", 0x7);
    disp_color_str("                                                               iWX:  7 ,MZ0B    ", 0x7);
    disp_color_str("                                                            M    ,    ,,WMMZ    ", 0x7);
    disp_color_str("                                                            r    ;B888aar SM7   ", 0x7);
    disp_color_str("                                                                  irXS          ", 0x7);
    disp_color_str("                                                                  X   :X        ", 0x7);
    disp_color_str("                                                                 S      S       ", 0x7);
    disp_color_str("                                                                                ", 0x7);
    disp_color_str("                                                                                ", 0x7);
}

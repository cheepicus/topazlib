
/* DO NOT use #include directives in this file */

#define Secure "/secure"
#define User  "/secure/user"
#define Auto "/secure/auto.inh"
#define Driver "/secure/driver.d"
#define UserDataDir "/secure/userdata/"

#define Obj "/obj"
#define Player "/obj/player"
#define WizSoul "/obj/wizsoul"
#define WizDir "/obj/wiz/"
#define WizHome(u) ("/wiz/" + (u))
#define Nowhere "/world/nowhere"

#define T_NIL      0
#define T_INT      1
#define T_FLOAT    2
#define T_STRING   3
#define T_OBJECT   4
#define T_ARRAY    5
#define T_MAPPING  6

#define IsNil(v) (typeof(v) == T_NIL)
#define IsInt(v) (typeof(v) == T_INT)
#define IsFloat(v) (typeof(v) == T_FLOAT)
#define IsString(v) (typeof(v) == T_STRING)
#define IsObject(v) (typeof(v) == T_OBJECT)
#define IsArray(v) (typeof(v) == T_ARRAY)
#define IsMapping(v) (typeof(v) == T_MAPPING)

#define IsAlpha(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define IsDigit(c) ((c) >= '0' && (c) <= '9')

#define Max(a,b) (((a)>(b)) ? (a) : (b) )
#define Min(a,b) (((a)>(b)) ? (b) : (a) )
#define Mid(a,b,c) (max(min((a),(b)), min(max((a),(b)),(c))))
#define Sign(x) ((x)<0 ? -1 : ((x)>0 ? 1 : 0))

#define Esc 			"\x1b"
#define Ansi(c)			("\x1b["+(c)+"m")
#define AnsiReset		"\x1b[m"
#define AnsiCls			"\x1b[2J\x1b[H"

#define AnsiBold		"\x1b[1m"
#define AnsiUnder		"\x1b[4m"
#define AnsiBlink		"\x1b[5m"
#define AnsiInverse		"\x1b[7m"

#define AnsiFGBlack 	"\x1b[30m"
#define AnsiFGDRed 		"\x1b[31m"
#define AnsiFGDGreen 	"\x1b[32m"
#define AnsiFGDYellow	"\x1b[33m"
#define AnsiFGDBlue 	"\x1b[34m"
#define AnsiFGDMagenta 	"\x1b[35m"
#define AnsiFGDCyan 	"\x1b[36m"
#define AnsiFGLGray 	"\x1b[37m"
#define AnsiFGDGray 	"\x1b[90m"
#define AnsiFGRed 		"\x1b[91m"
#define AnsiFGGreen 	"\x1b[92m"
#define AnsiFGYellow	"\x1b[93m"
#define AnsiFGBlue 		"\x1b[94m"
#define AnsiFGMagenta 	"\x1b[95m"
#define AnsiFGCyan 		"\x1b[96m"
#define AnsiFGWhite 	"\x1b[97m"

#define AnsiBGBlack 	"\x1b[40m"
#define AnsiBGDRed 		"\x1b[41m"
#define AnsiBGDGreen 	"\x1b[42m"
#define AnsiBGDYellow	"\x1b[43m"
#define AnsiBGDBlue 	"\x1b[44m"
#define AnsiBGDMagenta 	"\x1b[45m"
#define AnsiBGDCyan 	"\x1b[46m"
#define AnsiBGLGray 	"\x1b[47m"
#define AnsiBGDGray 	"\x1b[100m"
#define AnsiBGRed 		"\x1b[101m"
#define AnsiBGGreen 	"\x1b[102m"
#define AnsiBGYellow	"\x1b[103m"
#define AnsiBGBlue 		"\x1b[104m"
#define AnsiBGMagenta 	"\x1b[105m"
#define AnsiBGCyan 		"\x1b[106m"
#define AnsiBGWhite 	"\x1b[107m"

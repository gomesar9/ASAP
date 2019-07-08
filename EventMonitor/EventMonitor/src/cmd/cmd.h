#define MAX_CMD_OPT_STR_LEN 64

/* Enums */
// Event Monitor Command options
typedef enum _EM_CMD_TYPE {
	EM_CMD_START,	// 00
	EM_CMD_CFG,		// 01
	EM_CMD_STOP,	// 02
	EM_CMD_NULL		// 03
}EM_CMDTYPE, *PEM_CMDTYPE;

// Event Monitor Command Subtypes options
typedef enum _EM_SUBTYPE {
	EM_CFG_EVT,				// 00
	EM_CFG_COLLECT_MAX,		// 01
	EM_CFG_THRESHOLD,		// 02
	EM_CFG_COLLECT_MILLI,	// 03
	EM_EVT_NULL				// 04
}EM_SUBTYPE, *PEM_SUBTYPE;

// Core options
typedef enum _START_CFG {
	EM_STCFG_CORE0 = 1,
	EM_STCFG_CORE1 = 1<<1,
	EM_STCFG_CORE2 = 1<<2,
	EM_STCFG_CORE3 = 1<<3
}START_CFG, *PSTART_CFG;

/* Structs */
// Event Monitor Command
typedef struct st_EM_CMD {
	UINT32 Cores;
	EM_CMDTYPE Type;
	EM_SUBTYPE Subtype;
	INT Opt1, Opt2;
	CHAR[MAX_CMD_OPT_STR_LEN] Opt_str;
}TEM_CMD, *PTEM_CMD;

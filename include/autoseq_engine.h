/* ====================================================
 *  Public API implementation
 * ==================================================== */

#include <stdbool.h>
#include "decode_ft8.h"
#include "main.h"

//#define MAX_MSG_LEN 40
#define MAX_MSG_LEN 22
#define MAX_LINE_LEN 22
#define MAX_RX_ROWS 10
#define MAX_QSO_ROWS 10
#define MAX_QSO_ENTRIES 100
#define START_X_LEFT 0
#define START_X_RIGHT 300
#define START_Y 100 // FFT_H
#define LINE_HT 40
#define CALLSIGN_SIZE 10

void autoseq_init(const char *myCall, const char *myGrid);

void autoseq_start_cq(void);

/* === Called for selected decode (manual response) === */
void autoseq_on_touch(const Decode *msg);

/* === Called for **every** new decode (auto response) === */
/* Return whether TX is needed */
bool autoseq_on_decode(const Decode *msg);

/* === Provide the message we should transmit this slot (if any) === */
bool autoseq_get_next_tx(char out_text[MAX_MSG_LEN]);

/* === Populate the string for displaying the current QSO state  === */
void autoseq_get_qso_state(char out_text[MAX_LINE_LEN]);

/* === Slot timer / time‑out manager === */
void autoseq_tick(void);

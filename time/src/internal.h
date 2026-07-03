#ifndef COTER_TIME_INTERNAL_H
#define COTER_TIME_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

void ct_cron_mgr_process_once(void);
void ct_timer_mgr_process_once(void);

#ifdef __cplusplus
}
#endif

#endif /* COTER_TIME_INTERNAL_H */

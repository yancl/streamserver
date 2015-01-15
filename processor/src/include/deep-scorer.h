#ifndef __DEEP_SCORER_H__
#define __DEEP_SCORER_H__

#ifdef __cplusplus
extern "C" {
#endif
typedef struct DeepScorer DeepScorer;

DeepScorer* DeepScorerNew(char const *resource_type);

void DeepScorerDestroy(DeepScorer* ds);

int DeepScorerProcessRaw(DeepScorer *ds,
                         const short *buf,
                         size_t buf_size
                         );

void DeepScorerStart(DeepScorer *ds, char const *res_cfg, char const *ref_text);

void DeepScorerEnd(DeepScorer *ds);

char const* DeepScorerJsonOutput(DeepScorer *ds);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __DEEP_SCORER_H__ */

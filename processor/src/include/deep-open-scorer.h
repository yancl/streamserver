#ifndef __DEEP_OPEN_SCORER_H__
#define __DEEP_OPEN_SCORER_H__

#ifdef __cplusplus
extern "C" {
#endif
typedef struct DeepOpenScorer DeepOpenScorer;

DeepOpenScorer* DeepOpenScorerNew(char const *resource_type);

void DeepOpenScorerDestroy(DeepOpenScorer* dos);

int DeepOpenScorerProcessRaw(DeepOpenScorer *dos,
                         const short *buf,
                         size_t buf_size
                         );

void DeepOpenScorerStart(DeepOpenScorer *dos, char const *res_cfg);

void DeepOpenScorerEnd(DeepOpenScorer *dos);

char const* DeepOpenScorerJsonOutput(DeepOpenScorer *dos);

char const* DeepOpenScorerGetDecodedText(DeepOpenScorer *dos);

void DeepOpenScorerSetQid(DeepOpenScorer *dos, char const *qid);
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __DEEP_OPEN_SCORER_H__ */

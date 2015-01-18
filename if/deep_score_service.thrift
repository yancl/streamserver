namespace go deepscore
namespace cpp deepscore

enum ResultCode {
  OK,
  TRY_LATER
}

enum SliceFlag {
  START = 0,
  MIDDLE = 1,
  FINISH = 2,
  BROKEN = 10
}

struct DataSlice {
1: string key,
2: binary val,
3: i32 number,
4: SliceFlag flag,
5: string host,
6: i32 port
}

service DeepScorerService {
  ResultCode AddDataSliceStream(1: list<DataSlice> slices)
}

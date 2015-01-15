enum ResultCode
{
  OK,
  TRY_LATER
}

struct DataEntry {
1: string key,
2: binary val,
3: i32 number,
4: bool last = false
}

service DeepScorerService {
  ResultCode AddDataEntryStream(1: list<DataEntry> messages)
}

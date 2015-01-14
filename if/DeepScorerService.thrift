struct Message {
1: string key,
2: binary val,
3: i32 number
}

service DeepScorerService {
  void AddStreamDataSlice(1: Message msg)
}

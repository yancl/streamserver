#curl -v -include -H "Transfer-Encoding: chunked" --form session_key=100005 --form filename=@/home/yancl/downloads/boost_1_57_0.tar.bz2 http://localhost:8081/upload
curl -v -include -H "Transfer-Encoding: chunked" --form session_key=100001 --form filename=@test.txt http://localhost:8081/upload
#curl -v -include --form session_key=100001 --form filename=@uploads/test.txt http://localhost:8081/upload

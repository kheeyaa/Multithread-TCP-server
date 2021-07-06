시스템 프로그래밍 과제 - Multithread TCP server

- 서버 프로세스는 TCP 통신을 지원하며, 클라이언트의 요청의 응답하는 형식으로 동작한다.
- 서버 프로세스는 새로 접속하는 클라이언트 마다 각각 thread를 생성하여 서비스한다.
- 클라이언트는 최대 50개까지 접속할 수 있다고 가정한다.
- 서버 내부에는 다음과 같은 key-value 쌍으로 구성된 테이블을 가진다. 이때, 테이블 내
  항목의 갯수는 최대 100개, 각 key, value 문자열의 길이는 최대 50이며, 문자열은 공백없 이 연결된 한글 또는 영문 단어이다. (당연히, 각 항목의 초기 상태는 NULL을 담고 있다.) \* 아래 예는 기본 구현 예시이며, hash map, dictionary 등 기존 라이브러리 함수를 사용 해도 무방함.
  예: char kv[100][2][50] = { { "안성", "유기" }, { "이천", "쌀" }, { "나주", "배" }, { "대구", " 사과" }, { "Paris", "Eiffel-Tower" }, { "London", "Buckingham" }, ..... }
- 서버는 접속하는 클라이언트마다 번호(ID)를 부여한다. (1번부터 차례로 부여할 것)
- 각 클라이언트는 서버에게 다음과 같은 명령을 요청하고, 그에 대한 응답을 받는다.

클라이언트의 요청 형 식 (문자열)/의미/서버의 응답 형식 (문자열) (아래 ID는 서버가 클라이언트에게 부여 한 번호)

1. read key
   key에 대응하는 value를 요청
   ID read: key value
2. update key value
   key에 대응하는 값을 value로 변 경
   ID update OK : key value/ ID update Failed (key가 없는 경우)
3. insert key value
   (key, value) 쌍을 추가(이때 key는 중복되지 않는 새로운 값임)
   ID insert OK : key value/ ID insert Failed (kv 테이블에 더이상 공간이 없는 경우)

- 서버는 클라이언트로부터 받은 요청(문자열) 받으면, 다음과 같이 해당 client ID와 함께 표 준출력에 나타낸다.
  (예) client 01 : read 안성 -
  client 02 : update 안성 포도
- 클라이언트는 서버로부터 받은 응답(문자열)을 그대로 표준출력에 나타낸다.
  (예) 01 read: 안성 유기
- 각 클라이언트는 종료를 위해 “quit" 문자열을 server에게 보낸다. server는 이 문자열을
  받는 즉시 ”quit_ack"를 client에게 보내고, 해당 thread를 종료시킨다.
- 클라이언트는 “quit_ack"를 받으면 종료한다.

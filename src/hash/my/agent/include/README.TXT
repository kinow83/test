
1.로그 파일 DBMS로의 전송 시간 
  + 시 : 매 시
	+ 분 : 에이전트 번호 % moduler(31)
	+ 초 : log_send_sec(10초)
	


2.설정파일

<예시>
ID=3
DEV=eth0
MASTER=211.221.225.34
PASSWORD=1234
URL_PATH=html/block_page.html

	+ ID : 에이전트 번호
	+ DEV : 미러링 이더넷 장치 이름(리눅스 환경)
	+ MASTER : DBMS 서버 IP 주소
	+ PASSWORD : DBMS 접속시 비밀 번호
	+ URL_PATH : 페이크 페이지를 보여줄 html(php...) 웹 페이지 저장 루트
						: 비어 있을 경우 디폴트 'html/block_page.html'로 지정됨.



3.new_cata
new_cata 프로그램은 카테고리의 URL을 생성, 추가, 삭제 한다

<예시>
┌─────────────────┐
│             MENU                 │
│ 1. Create Base File              │
│ 2. New Policy                    │
│ 3. Add Policy                    │
│ 4. Del Policy                    │
│ 5. EXIT                          │
└─────────────────┘
select>>

	+ select>>1	: new_db 디렉토리에 카테고리 항목 파일이 생성되며,
							:	해당 카테고리 파일에 URL을 추가한다.
							: 카테고리 목록
							: [카테고리 항목]
							:	adult     culture  education  etc   international  living  organization  politics  shop     sports
								computer  economy  entertain  game  internet       media   p2p           religion  society  tour
								
	+ select>>2	: DBMS(디비 서버)의 policy DB 생성
							: 카테고리 항목의 URL을 디비에 추가한다.
							
	+ select>>3	:	카테고리 항목의 URL을 디비에 추가한다.
	
	+ select>>4	: DBMS(디비 서버)의 policy DB를 삭제한다.
	
	+ select>>5	: 종료
	
	
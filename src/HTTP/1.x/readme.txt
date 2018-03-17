1. connection
	- HTTP 1.0: Connection: close (default)
	- HTTP 1.1: Connection: Keep-Alive (default) - persist
				Connection: close <- force close
	이슈: Persistent Connection을 사용하지 않는 경우
			서버에 연결된 모든 클라이언트의 TCP연결이 계속 늘어나다보면
			서버의 자원이 고갈되어 더울 많은 클라이언트의 접속에 대처할 수 없는 상황 시나리ㅗ
			따라서 클라이언트의 접속이 제일 잦은 메인 페이지와 같은 URL에서는 
			서버의 가용성을 고려하여 Persistent Connection을 사용할지 고민...

			반대로 Persistent connetion을 사용하면 얻을 수 있는 장점은
			서버의 단일 시간내 TCP연결의 수를 그 만큼 적게함. 서버 CPU, 메모리 자원 절약,
			네트워크 혼잡이나 지연의 경의 수를 줄임.
2. pipelining
	- HTTP 1.1
	- 다수의 HTTP 요청과 응답이 클라이언트와 서버 사이에 응답지연(HOL)없이
		stream형태로 주고 받을 수 있는 기술
	- 지원 메서드: GET, HEAD, PUT, DELETE
	이슈: 프록시와 서버가 지원해야 하는 문제...

3. 도메인 샤딩

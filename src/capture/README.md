
ARP 패킷 변조 테스트

[case 1]
sender ip  = 10.10.200.150
sender mac = 00:00:00:00:00:00
target ip  = 10.10.200.1
target mac = ff:ff:ff:ff:ff:ff

이슈: 다른 호스트(특히 10.10.200.1)는 ARP 캐시 테이블에 10.10.200.150를 00:00:00:00:00:00으로 갱신해버림.
      호스트가 router인 경우 통신 두절 가능성


[case 2]
sender ip  = 0.0.0.0
sender mac = 00:00:00:00:00:00
target ip  = 10.10.200.1
target mac = ff:ff:ff:ff:ff:ff

이슈: Windows, OSX, Linux 등의 호스트는 ARP Reply 보내는거 같음.
      하지만 router(또는 네트워크장비)는 ARP Reply를 보내지 않아 탐지에 구멍이 생김.

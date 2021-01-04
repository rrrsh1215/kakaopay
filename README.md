[개발 프레임워크]
* 사양
	- OS: centos(8.2)
	- 자바 (1.8.0_272)
	- DB: sqlite3(snapshot-202012141539)
	- 언어: C, json-c(0.13.1)
	- 암복호화: openssl(1.1.1g) - seed 암복호화

* 범용 라이브러리 직접 제작
	- castutil : 형 변환 등 기능 생성
	- cfgutil  : config 파일 Load
	- dbutil   : sqlite3 에서 사용되는 함수를 래핑
	- seedutil : 암복호화 등 기능 생성
	- timeutil : 현재시간 조회 등 기능 생성

* 디렉토리 구조
- bin         : 실행 프로그램
- cfg         : config 파일
- database    : sqlite3 database
- inc         : header 파일
- jsonfile    : 요청 전문(req) 및 응답 전문(res) 저장 (json 파일 형태)
- lib         : 범용 라이브러리
	- bin         : 컴파일 된 라이브러리(*.a)
	- src         : 컴파일 소스
		- castutil     
		- cfgutil      
		- dbutil       
		- seedutil     
		- timeutil     

- opensource : 오픈소스
	- include : header 파일
		- json-c
		- openssl
		- sqlite3
	- lib     : 라이브러리 파일(*.a   *.so)
	- private : seedkey
- shl : 실행 프로그램을 가동하는 shell 프로그램
- src : 프로그램 소스

- .bash_profile  : "export SH_HOME" 변수에 설치하고자 하는 home directory 지정 필수. ex) export SH_HOME=~


[테이블 설계]
01. TB_PAYINFO_STRING
	- REQ_DATA CHAR(450) : 요청전문을 생성하여 스트링형태로 저장

02. TB_PAYINFO
	- MNG_NO      CHAR(20) PRIMARY KEY  : 관리 번호. 식별자
	- REQ_CD      VARCHAR2(10)          : 데이터 구분(PAYMENT: 결제 및 취소 후 잔존금액 표시, CANCEL: 취소금액 표시)
	- ISTM_CD     CHAR(2)               : 할부 개월수
	- PAY_AMT     NUMBER(10)            : 거래금액 (PAYMENT: 결제 금액, CANCEL: 취소 금액)
	- VAT         NUMBER(10)            : 부가가치세 (PAYMENT: 결제 금액의 VAT, CANCEL: 취소 금액의 VAT)
	- ORG_MNG_NO  CHAR(20)              : 원거래 관리번호 (CANCEL: 원거래 관리번호가 동일한 거래금액의 합계 = 총 취소금액)
	- ENC_INFO    VARCHAR2(300)         : 암호화된 카드정보 (PAYMENT: 카드번호|유효기간|CVC  정보 암호화)
	- SYSDTTM     CHAR(15)              : 처리일시 (YYYYMMDD HHMISS).
		- 동시 결제/취소 여부를 확인하기 위해 사용.
		- 동일 카드번호로 TIMEGAP(config.cfg 파일에서 설정) 이내에 연속 결제/취소 요청시 동시로 보고 처리를 반려함.

[문제해결 전략]
00. 공통
- 관리번호 생성 (00000000000000000001 부터 zzzzzzzzzzzzzzzzzzzz 까지 1 씩 증가)
	- 관리번호를 관리하는 파일을 생성한다. ($SH_HOME/cfg/mng_no.dat)
	- 관리번호로부터 마지막으로 생성된 관리번호를 조회
	- 최종 관리번호에 1을 더한 값을 신규 관리번호로 생성함. (단, 0~9 -> A~Z -> a->z 순으로 증가시킨다.)
	- 신규 관리번호를 다시 관리번호 파일에 저장함.
	- 추후 PTHREAD 환경에서는, 파일 락등을 이용해서 식별자 중복 오류를 방지해야 한다.


01. POST: 결제 요청
	- json 파일 형태의 요청 정보를 읽음

	- 해당 정보를 구조체 형태로 변환하여 저장

	- 결제 요청 일시 조회
	
	- 신규 관리번호 생성

	- 카드번호|유효기간|CVC  정보를 SEED 암호화 처리

	- TB_PAYINFO_STRING 테이블에 스트링 형태로 저장.

	- 기존에 동일한 카드로 TIMEGAP(config.cfg 파일에서 설정) 이내 결제 요청이 있었는지 확인 (shryu: 구현 예정)
		- 동시 결제 요청이 있는 경우 반려 처리
		- 동시 결제 요청이 없는 경우 TB_PAYINFO 테이블에 데이터 구분을 PAYMENT 로 하여 저장.

	- 응답 전문(res) 생성
		- 성공시 코드(00), 실패시 코드(01)
		

02. DELETE: 전체취소 요청
	- json 파일 형태의 요청 정보를 읽음

	- 해당 정보를 구조체 형태로 변환하여 저장

	- 결제 요청 일시 조회

	- 신규 관리번호 생성

	- TB_PAYINFO_STRING 테이블에 스트링 형태로 저장.

	- 최근에 동일한 카드로 결제(PAYMENT) 된 내역이 있었는지 확인 (shryu: 구현 예정)
		- 없는 경우 반려 처리
		- 있는 경우
			- 잔존 거래금액과 취소 금액이 일치하지않으면 반려 처리.

			- TIMEGAP(config.cfg 파일에서 설정) 이내 취소 요청이 있었는지 확인 
				- 있는 경우 반려 처리
				- 없는 경우 PAYMENT 로 저장된 정보에서 '잔존 결제금액 = 0'. '잔존 VAT = 0' 으로 변경
				- TB_PAYINFO 테이블에 데이터 구분을 CANCEL 로 하여 저장.

	- 응답 전문(res) 생성
		- 성공시 코드(00), 실패시 코드(01)

03. PUT: 부분취소 요청				
	- json 파일 형태의 요청 정보를 읽음

	- 해당 정보를 구조체 형태로 변환하여 저장

	- 결제 요청 일시 조회

	- 신규 관리번호 생성

	- TB_PAYINFO_STRING 테이블에 스트링 형태로 저장.

	- 최근에 동일한 카드로 결제(PAYMENT) 된 내역이 있었는지 확인 (shryu: 구현 예정)
		- 없는 경우 반려 처리
		- 있는 경우
			- TIMEGAP(config.cfg 파일에서 설정) 이내 취소 요청이 있었는지 확인 
				- 있는 경우 반려 처리
				- 없는 경우
					- 잔존 결제금액 < 취소금액이면 반려 처리

					- 잔존 결제금액 = 취소금액이면
						- 취소 VAT 금액이 없는 경우
							- 취소 VAT 금액 = 잔존 VAT 금액 으로 하여 취소 처리

						- 취소 VAT 금액이 있는 경우
							- 잔존 VAT 금액 != 취소 VAT 금액 이면  반려 처리
							- 잔존 VAT 금액 == 취소 VAT 금액 이면  취소 처리

					- 잔존 결제금액 > 취소금액이면
						- 취소 VAT 금액이 없는 경우, 취소 VAT 금액 = 취소금액 / 11
						
						- 잔존 VAT 금액 <  취소 VAT 금액 이면  반려 처리
						- 잔존 VAT 금액 >= 취소 VAT 금액 이면  취소 처리

	- 응답 전문(res) 생성
		- 성공시 코드(00), 실패시 코드(01)

04. GET: (잔존) 결제 금액 정보 조회
	- json 파일 형태의 요청 정보를 읽음

	- 해당 정보를 구조체 형태로 변환하여 저장

	- 관리번호를 식별자로 하여 (잔존) 결제 금액 정보 조회

	- 응답 전문(res) 생성
		- 성공시 코드(00), 실패시 코드(01)

[빌드 방법]
01. 라이브러리 전체 컴파일
	$SH_HOME/lib/src ] make -f MAKEFILE
	
	--> $SH_HOME/lib/bin 디렉토리에 각종 라이브러리(*.a) 생성됨.

02. 프로그램 컴파일
	$SH_HOME/src ] make
	
	--> $SH_HOME/bin 디렉토리에 실행파일(client)이 생성 됨.


[실행 방법]
	- 결제      : $SH_HOME/shl ] client.sh POST    요청번호(EX: 00, 01, 02 ...)
	- 전체 취소 : $SH_HOME/shl ] client.sh DELETE  요청번호(EX: 00, 01, 02 ...)
	- 부분 취소 : $SH_HOME/shl ] client.sh PUT     요청번호(EX: 00, 01, 02 ...)
	- 조회      : $SH_HOME/shl ] client.sh GET     요청번호(EX: 00, 01, 02 ...)

! 요청번호란
	- $SH_HOME/jsonfile  에 존재하는 요청전문(req.json) 파일의 앞에 두자리 숫자  ex) 00[00_req.json]   01[01_req.json] ...
	- 프로그램 실행 시 해당 요청번호의 응답전문(res.json) 파일을 생성 한다.      ex) 00[00_res.json]   01[01_res.json] ...

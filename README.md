# license_generator(라이센스 생성 프로그램)과 LibLicense (라이센스 검증 라이브러리)

라이센스 파일(.license)을 통해 애플리케이션 사용 기간을 검증하는 경량 C 라이브러리. 

라이센스 파일을 로드하고, 무결성을 검증하며, 만료 여부를 빠르게 확인할 수 있도록 설계. 
OpenSSL을 활용해 SHA-256 해시 및 AES-256-CBC 복호화를 수행.

---
## 1. 프로젝트 개요

<table style="border-collapse:collapse;width:100%;font-size:15px;">
<thead>
<tr style="background:#001f4d;color:white;">
<th style="border:1px solid #bbb;padding:8px 16px;text-align:center;">구분</th>
<th style="border:1px solid #bbb;padding:8px 16px;text-align:center;">설명</th>
</tr>
</thead>
<tbody>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">목적</td><td style="border:1px solid #bbb;padding:8px 16px;">배포 애플리케이션의 사용 허용 기간/환경을 간단히 검증</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">라이센스 파일</td><td style="border:1px solid #bbb;padding:8px 16px;"><code>$HOME/data/.license</code> (SHA256 + AES 암호문 구조)</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">무결성 방식</td><td style="border:1px solid #bbb;padding:8px 16px;">MAC + 암호 데이터 + UUID 결합 후 SHA-256 서명 비교</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">복호화</td><td style="border:1px solid #bbb;padding:8px 16px;">AES-256-CBC (OpenSSL EVP)</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">지원 OS</td><td style="border:1px solid #bbb;padding:8px 16px;">Linux (현재 POSIX 경로/<code>/sys</code> 의존)</td></tr>
</tbody>
</table>

### * 구조 요약:
1. `load_license_file()` 호출 → 라이센스 파일 읽기
2. MAC / UUID 수집 → 서명 재계산(SHA-256)
3. 저장된 서명과 비교 → AES 복호화
4. `Crypt_info`(요청/만료 시각) 메모리에 적재
5. 초고속 조회 API 사용
    * `license_error_code_to_string()`
    * `is_license_valid_period()`
    * `get_license_expire_time()`
    * `get_license_create_date()`

---
## 2. 디렉터리 구조

```shell
include/liblicense.h        공개 헤더
libsrc/liblicense.c         라이브러리 구현
tools/license_generator.c   라이센스 생성 도구
data/.license               샘플 라이센스 파일 (make 후 생성)
bin/main_test               성능 및 기능 검증 실행 파일 (make 후 생성)
lib/liblicense.a            정적 라이브러리 산출물 (make 후 생성)
```

---
## 3. 빌드 & 실행 방법

프로젝트 빌드:
```bash
cd ~/library_root
make # 라이브러리 + 테스트 바이너리 생성
```

성능/기능 테스트 실행:
```bash
./tools/license_generator # -h 옵션 -> usage
./bin/main_test
```

애플리케이션에서 사용 예 (요약):
```c
#include "liblicense.h"

int main(void) {
	if (load_license_file() != License_result_success) {
		fprintf(stderr, "License load failed: %s\n", license_error_code_to_string(License_file_not_found_error));
		return 1;
	}
	if (!is_license_valid_period()) {
		fprintf(stderr, "License expired.\n");
		return 1;
	}
	time_t expire = get_license_expire_time();
	if (expire != 0) {
		printf("Expire at: %ld\n", (long)expire);
	} else {
		printf("Unlimited license.\n");
	}
	return 0;
}
```

---
## 4. 공개 데이터 구조

`Crypt_info` (라이센스 핵심):
```c
typedef struct __attribute__((__packed__)) {
	time_t request_time;  // 발급(생성) 시간
	time_t expire_time;   // 만료 시간 (0이면 무제한)
} Crypt_info;
```

`License_generator_info` (내부 상태 – 전역 `info`):
```c
typedef struct {
	Crypt_info crypt_info;              // 복호화된 라이센스 정보
	int        req_time;                // (미사용 / 확장 필드 가능)
	unsigned char signature_sha256[32]; // 재계산된 서명
	unsigned char compare_sha256[32];   // 라이센스 파일에 저장된 서명
	unsigned char aes_bin[...] ;        // 암호문
	char uuid[37];                      // 하드웨어 UUID
	char mac[18];                       // MAC 주소
	int aes_len;                        // 암호문 길이
} License_generator_info;
```

---
## 5. 에러 코드

<table style="border-collapse:collapse;width:100%;font-size:15px;">
<thead>
<tr style="background:#001f4d;color:white;">
<th style="border:1px solid #bbb;padding:8px 16px;text-align:center;">코드</th>
<th style="border:1px solid #bbb;padding:8px 16px;text-align:center;">의미 (요약)</th>
</tr>
</thead>
<tbody>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">License_result_success</td><td style="border:1px solid #bbb;padding:8px 16px;">성공</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">Sha256_*</td><td style="border:1px solid #bbb;padding:8px 16px;">SHA-256 초기화/갱신/최종 처리 실패</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">Aes256_*</td><td style="border:1px solid #bbb;padding:8px 16px;">AES 키 길이/컨텍스트/복호화 단계 실패</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">Env_home_error</td><td style="border:1px solid #bbb;padding:8px 16px;">HOME 환경변수 미설정</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">License_file_not_found_error</td><td style="border:1px solid #bbb;padding:8px 16px;">라이센스 파일 없음</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">License_file_read_error</td><td style="border:1px solid #bbb;padding:8px 16px;">파일 읽기 실패</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">License_invalid_error</td><td style="border:1px solid #bbb;padding:8px 16px;">서명 불일치 또는 복호화 실패</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">Set_mac_error</td><td style="border:1px solid #bbb;padding:8px 16px;">MAC 주소 수집 실패</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">License_invalid_parameter_error</td><td style="border:1px solid #bbb;padding:8px 16px;">내부 파라미터 검증 실패</td></tr>
</tbody>
</table>

문자열 변환: `license_error_code_to_string(code)`

---
## 6. 공개 API 요약

<table style="border-collapse:collapse;width:100%;font-size:15px;">
<thead>
<tr style="background:#001f4d;color:white;">
<th style="border:1px solid #bbb;16px;text-align:center;">함수</th>
<th style="border:1px solid #bbb;32;text-align:center;">반환</th>
<th style="border:1px solid #bbb;120px;text-align:center;">설명</th>
<th style="border:1px solid #bbb;16px;text-align:center;">사전 조건</th>
</tr>
</thead>
<tbody>
<tr><td style="border:1px solid #bbb;padding:8px 16px;"><code>License_error_code load_license_file(void)</code></td><td style="border:1px solid #bbb;padding:8px 16px;">에러 코드</td><td style="border:1px solid #bbb;padding:8px 16px;">라이센스 파일 로드 및 검증</td><td style="border:1px solid #bbb;padding:8px 16px;"><code>최초 1회 필수</code> 이후 라이센스 파일 확인 필요시</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;"><code>int is_license_valid_period(void)</code></td><td style="border:1px solid #bbb;padding:8px 16px;">1/0</td><td style="border:1px solid #bbb;padding:8px 16px;">현재 시간이 만료 이전인지 검사</td><td style="border:1px solid #bbb;padding:8px 16px;"><code>load_license_file()</code> 성공 후</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;"><code>time_t get_license_expire_time(void)</code></td><td style="border:1px solid #bbb;padding:8px 16px;">time_t</td><td style="border:1px solid #bbb;padding:8px 16px;">만료 시간 (0=무제한)</td><td style="border:1px solid #bbb;padding:8px 16px;">로드 후</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;"><code>time_t get_license_create_date(void)</code></td><td style="border:1px solid #bbb;padding:8px 16px;">time_t</td><td style="border:1px solid #bbb;padding:8px 16px;">발급(요청) 시간</td><td style="border:1px solid #bbb;padding:8px 16px;">로드 후</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;"><code>const char* license_error_code_to_string(code)</code></td><td style="border:1px solid #bbb;padding:8px 16px;">문자열</td><td style="border:1px solid #bbb;padding:8px 16px;">에러 코드→문자열</td><td style="border:1px solid #bbb;padding:8px 16px;">항상 사용 가능</td></tr>
</tbody>
</table>

---

참고: `load_license_file()`는 실행 시 1회 호출 후 만료일을 캐시.

---
## 7. 내부 처리 흐름 (상세)

1. 경로 설정: `$HOME/data/.license` 열기
2. 파일 헤더 → 저장된 SHA256(32바이트) 추출
3. 나머지 구간 → AES 암호문 저장
4. 시스템에서 MAC/UUID 수집
5. (MAC + 암호문 + UUID) → SHA-256 재계산 → 비교
6. 동일 시 AES-256-CBC 복호화 → `Crypt_info` 적재
7. 만료 시간 확인 후 API 노출

---
## 8. 보안 및 운영 상 주의사항

<table style="border-collapse:collapse;width:100%;font-size:15px;">
<thead>
<tr style="background:#001f4d;color:white;">
<th style="border:1px solid #bbb;padding:8px 16px;text-align:center;">항목</th>
<th style="border:1px solid #bbb;padding:8px 16px;text-align:center;">설명</th>
<th style="border:1px solid #bbb;padding:8px 16px;text-align:center;">권장 조치</th>
</tr>
</thead>
<tbody>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">키/패스프레이즈</td><td style="border:1px solid #bbb;padding:8px 16px;">현재 코드에 상수 포함</td><td style="border:1px solid #bbb;padding:8px 16px;">회의 후 변경 필요</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">OpenSSL 버전</td><td style="border:1px solid #bbb;padding:8px 16px;">1.0.x vs 3.x API 차이</td><td style="border:1px solid #bbb;padding:8px 16px;">CI에서 다중 버전 빌드 테스트</td></tr>
<tr><td style="border:1px solid #bbb;padding:8px 16px;">linux 하드웨어 종속</td><td style="border:1px solid #bbb;padding:8px 16px;"><code>/sys</code> 경로 사용</td><td style="border:1px solid #bbb;padding:8px 16px;">Windows / macOS 포팅 시 추상화 계층 추가</td></tr>
</tbody>
</table>

---
## 9. FAQ (요약)

Q. 라이센스가 무제한인지 어떻게 알 수 있나요?  

A. `get_license_expire_time()` 반환값이 0이면 무제한입니다.

Q. 최초 호출 순서는?

A. `load_license_file()` → (성공 시) 나머지 API 호출.

---
작성일: 2025-08-29


#include <pgmspace.h>

int send_readings = 0;
long interval = 1000;
float accX;
float accY;
float accZ;
long ttime;

#define SECRET
#define THINGNAME "CLIENT_NAME_AS_IT_APPEARS_IN_AWS_POLICY"

const char WIFI_SSID[] = "YOUR_SSID";
const char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
const char AWS_IOT_ENDPOINT[] = "AWS_IOT_ENDPOINT";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
REPLACE THIS WITH CERTIFICATE CONTENTS
-----END CERTIFICATE-----
)EOF";

// Device Certificate 
// m5-stick-c-plus certificates
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
REPLACE THIS WITH CERTIFICATE CONTENTS
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
REPLACE THIS WITH CERTIFICATE CONTENTS
-----END RSA PRIVATE KEY-----
)KEY";

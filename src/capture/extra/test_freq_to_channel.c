#include <stdio.h>
#include <stdlib.h>


int IEEE80211Freq[][2] = {
	{1, 2412},
	{2, 2417},
	{3, 2422},
	{4, 2427},
	{5, 2432},
	{6, 2437},
	{7, 2442},
	{8, 2447},
	{9, 2452},
	{10, 2457},
	{11, 2462},
	{12, 2467},
	{13, 2472},
	{14, 2484},
	{36, 5180},
	{37, 5185},
	{38, 5190},
	{39, 5195},
	{40, 5200},
	{41, 5205},
	{42, 5210},
	{43, 5215},
	{44, 5220},
	{45, 5225},
	{46, 5230},
	{47, 5235},
	{48, 5240},
	{52, 5260},
	{53, 5265},
	{54, 5270},
	{55, 5275},
	{56, 5280},
	{57, 5285},
	{58, 5290},
	{59, 5295},
	{60, 5300},
	{64, 5320},
	{100, 5500},
	{104, 5520},
	{108, 5540},
	{112, 5560},
	{116, 5580},
	{120, 5600},
	{124, 5620},
	{128, 5640},
	{132, 5660},
	{136, 5680},
	{140, 5700},
	{149, 5745},
	{150, 5750},
	{152, 5760},
	{153, 5765},
	{157, 5785},
	{160, 5800},
	{161, 5805},
	{165, 5825},
	{0, 0}
};

// case 1
int FreqToChan1(int in_freq)
{
    int x = 0;
    // 80211b frequencies to channels

    while (IEEE80211Freq[x][1] != 0) {
        if (IEEE80211Freq[x][1] == in_freq) {
            return IEEE80211Freq[x][0];
        }
        x++;
    }

    return in_freq;
}

// case 2
int FreqToChan2(int freq)
{
	if (freq == 2484) {
		return 14;
	} else if (freq < 5000) {
		return ((freq - 2412) / 5) + 1;
	} else {
		return ((freq - 5180) / 5) + 36;
	}
}

// case 3
int FreqToChan3(int freq)
{
	switch (freq) {
	case 2412: return 1;
	case 2417: return 2;
	case 2422: return 3;
	case 2427: return 4;
	case 2432: return 5;
	case 2437: return 6;
	case 2442: return 7;
	case 2447: return 8;
	case 2452: return 9;
	case 2457: return 10;
	case 2462: return 11;
	case 2467: return 12;
	case 2472: return 13;
	case 2484: return 14;

	case 5180: return 36;
	case 5185: return 37;
	case 5190: return 38;
	case 5195: return 39;
	case 5200: return 40;
	case 5205: return 41;
	case 5210: return 42;
	case 5215: return 43;
	case 5220: return 44;
	case 5225: return 45;
	case 5230: return 46;
	case 5235: return 47;
	case 5240: return 48;
	case 5260: return 52;
	case 5265: return 53;
	case 5270: return 54;
	case 5275: return 55;
	case 5280: return 56;
	case 5285: return 57;
	case 5290: return 58;
	case 5295: return 59;
	case 5300: return 60;
	case 5320: return 64;
	case 5500: return 100;
	case 5520: return 104;
	case 5540: return 108;
	case 5560: return 112;
	case 5580: return 116;
	case 5600: return 120;
	case 5620: return 124;
	case 5640: return 128;
	case 5660: return 132;
	case 5680: return 136;
	case 5700: return 140;
	case 5745: return 149;
	case 5750: return 150;
	case 5760: return 152;
	case 5765: return 153;
	case 5785: return 157;
	case 5800: return 160;
	case 5805: return 161;
	case 5825: return 165;
	}
}

int main(int a, char**b) {
	int k1, k2;
	int N = 1000000;
	int ch;
	int type = atoi(b[1]);

	for (k1=0; k1<N; k1++) {
		for (k2=0; k2<57; k2++) {
			if (type == 1) {
				ch = FreqToChan1(IEEE80211Freq[k2][1]);
			} else if (type == 2) {
				ch = FreqToChan2(IEEE80211Freq[k2][1]);
			} else if (type == 3) {
				ch = FreqToChan3(IEEE80211Freq[k2][1]);
			}
//			printf("%d <- %d\n", ch, IEEE80211Freq[k2][1]);
		}
	}
}

#if 0
// result of test

kinow@kinow ~/$ time ./ban 1

real	0m3.071s
user	0m3.068s
sys	0m0.000s
kinow@kinow ~/$ time ./ban 2

real	0m0.236s
user	0m0.232s
sys	0m0.000s
kinow@kinow ~/$ time ./ban 3

real	0m0.278s
user	0m0.276s
sys	0m0.000s

#endif

#if !defined ( EV_HLDMH )
#define EV_HLDMH

enum disc_e 
{
	DISC_IDLE = 0,
	DISC_FIDGET,
	DISC_PINPULL,
	DISC_THROW1,	// toss
	DISC_THROW2,	// medium
	DISC_THROW3,	// hard
	DISC_HOLSTER,
	DISC_DRAW
};

#endif // EV_HLDMH
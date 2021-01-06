
class CHook : public CBaseEntity
{
	void Spawn( void );
	int Classify( void );
	void Precache( void );
	void EXPORT HookTouch( CBaseEntity *pOther );
	void Think ( void );
	Vector m_vPlayerHangOrigin;
	BOOL m_fPlayerAtEnd;
	short ropesprite;
	BOOL m_fHookInWall;
	BOOL m_fActiveHook;
	Vector m_vVecDirHookMove;
	CBasePlayer *pevOwner;

public:
	static CHook *HookCreate( CBasePlayer *owner );
	void FireHook( void );
	void KillHook( void );
};

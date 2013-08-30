// simple_state_machine.h
// Simple finite state machine encapsulation
// Author: Michael S. Booth (mike@turtlerockstudios.com), November 2003

#ifndef _SIMPLE_STATE_MACHINE_H_
#define _SIMPLE_STATE_MACHINE_H_

//--------------------------------------------------------------------------------------------------------
/**
 * Encapsulation of a finite-state-machine state 
 */
template < typename T >
class SimpleState
{
public:
	SimpleState( void )
	{
		m_parent = NULL;
	}

	virtual ~SimpleState() { }

	virtual void OnEnter( T userData ) { }			///< when state is entered
	virtual void OnUpdate( T userData ) { }			///< state behavior
	virtual void OnExit( T userData ) { }			///< when state exited
	virtual const char *GetName( void ) const = 0;	///< return state name

	void SetParent( SimpleState<T> *parent )	{ m_parent = parent; }
	SimpleState<T> *GetParent( void ) const		{ return m_parent; }

private:
	SimpleState<T> *m_parent;						///< the parent state that contains this state
};

//--------------------------------------------------------------------------------------------------------
/**
 * Encapsulation of a finite state machine
 */
template < typename T, typename S >
class SimpleStateMachine
{
public:
	SimpleStateMachine( void )
	{
		m_state = NULL;
	}

	void Reset( T userData )
	{
		m_userData = userData;
		m_state = NULL;
	}

	/// change behavior state - WARNING: not re-entrant. Do not SetState() from within OnEnter() or OnExit()
	void SetState( S *newState )
	{
		if (m_state)
			m_state->OnExit( m_userData );

		newState->OnEnter( m_userData );

		m_state = newState;
		m_stateTimer.Start();
	}

	float GetStateDuration( void ) const	{ return m_stateTimer.GetElapsedTime(); }	///< how long have we been in the current state

	bool IsState( const S *state ) const	{ return (state == m_state); }		///< return true if given state is current state of machine

	/// execute current state of machine
	void Update( void )
	{
		if (m_state)
			m_state->OnUpdate( m_userData );
	}

protected:
	S *m_state;												///< current behavior state
	IntervalTimer m_stateTimer;								///< how long have we been in the current state
	T m_userData;
};


#endif // _SIMPLE_STATE_MACHINE_H_


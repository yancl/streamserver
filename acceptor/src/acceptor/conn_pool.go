package acceptor

import (
	"container/list"
	"errors"
	"sync"
	"time"
)

var nowFunc = time.Now // for testing

// ErrConnPoolExhausted is returned from a pool connection method (Do, Send,
// Receive, Flush, Err) when the maximum number of database connections in the
// pool has been reached.
var ErrConnPoolExhausted = errors.New("acceptor: connection pool exhausted")

var (
	errConnPoolClosed = errors.New("acceptor: connection pool closed")
	errConnClosed     = errors.New("acceptor: connection closed")
)

type ConnPool struct {

	// Dial is an application supplied function for creating and configuring a
	// connection
	Dial func(addr string) (*ThriftConn, error)

	// TestOnBorrow is an optional application supplied function for checking
	// the health of an idle connection before the connection is used again by
	// the application. Argument t is the time that the connection was returned
	// to the pool. If the function returns an error, then the connection is
	// closed.
	TestOnBorrow func(c *ThriftConn, t time.Time) error

	//the pool only for the addr
	Addr string

	// Maximum number of connections allocated by the pool at a given time.
	// When zero, there is no limit on the number of connections in the pool.
	MaxActive int

	// Maximum number of idle connections in the pool.
	MaxIdle int

	// Close connections after remaining idle for this duration. If the value
	// is zero, then idle connections are not closed. Applications should set
	// the timeout to a value less than the server's timeout.
	IdleTimeout time.Duration

	// If Wait is true and the pool is at the MaxIdle limit, then Get() waits
	// for a connection to be returned to the pool before returning.
	Wait bool

	// mu protects fields defined below.
	mu     sync.Mutex
	cond   *sync.Cond
	closed bool
	active int

	// Stack of idleinterface{} with most recently used at the front.
	idle list.List
}

type idleConn struct {
	c *ThriftConn
	t time.Time
}

// NewConnPool creates a new pool. This function is deprecated. Applications should
// initialize the *ConnPool fields directly as shown in example.
func NewConnPool(newFn func(addr string) (*ThriftConn, error), addr string, maxActive int, maxIdle int, wait bool) *ConnPool {
	return &ConnPool{Dial: newFn, Addr: addr, MaxActive: maxActive, MaxIdle: maxIdle, Wait: wait}
}

// Get gets a connection. The application must close the returned connection.
// This method always returns a valid connection so that applications can defer
// error handling to the first use of the connection. If there is an error
// getting an underlying connection, then the connection Err, Do, Send, Flush
// and Receive methods return that error.
func (p *ConnPool) Get() (*ThriftConn, error) {
	c, err := p.get()
	if err != nil {
		//return errorConnection{err}
		return nil, err
	}
	return c, nil
	//return &pooledConnection{p: p, c: c}, nil
}

// ActiveCount returns the number of active connections in the pool.
func (p *ConnPool) ActiveCount() int {
	p.mu.Lock()
	active := p.active
	p.mu.Unlock()
	return active
}

// Close releases the resources used by the pool.
func (p *ConnPool) Close() error {
	p.mu.Lock()
	idle := p.idle
	p.idle.Init()
	p.closed = true
	p.active -= idle.Len()
	if p.cond != nil {
		p.cond.Broadcast()
	}
	p.mu.Unlock()
	for e := idle.Front(); e != nil; e = e.Next() {
		e.Value.(idleConn).c.Close()
	}
	return nil
}

// release decrements the active count and signals waiters. The caller must
// hold p.mu during the call.
func (p *ConnPool) release() {
	p.active -= 1
	if p.cond != nil {
		p.cond.Signal()
	}
}

// get prunes stale connections and returns a connection from the idle list or
// creates a new connection.
func (p *ConnPool) get() (*ThriftConn, error) {
	p.mu.Lock()

	// Prune stale connections.

	if timeout := p.IdleTimeout; timeout > 0 {
		for i, n := 0, p.idle.Len(); i < n; i++ {
			e := p.idle.Back()
			if e == nil {
				break
			}
			ic := e.Value.(idleConn)
			if ic.t.Add(timeout).After(nowFunc()) {
				break
			}
			p.idle.Remove(e)
			p.release()
			p.mu.Unlock()
			ic.c.Close()
			p.mu.Lock()
		}
	}

	for {

		// Get idle connection.

		for i, n := 0, p.idle.Len(); i < n; i++ {
			e := p.idle.Front()
			if e == nil {
				break
			}
			ic := e.Value.(idleConn)
			p.idle.Remove(e)
			test := p.TestOnBorrow
			p.mu.Unlock()
			if test == nil || test(ic.c, ic.t) == nil {
				return ic.c, nil
			}
			ic.c.Close()
			p.mu.Lock()
			p.release()
		}

		// Check for pool closed before dialing a new connection.

		if p.closed {
			p.mu.Unlock()
			return nil, errors.New("redigo: get on closed pool")
		}

		// Dial new connection if under limit.

		if p.MaxActive == 0 || p.active < p.MaxActive {
			dial := p.Dial
			p.active += 1
			p.mu.Unlock()
			c, err := dial(p.Addr)
			if err != nil {
				p.mu.Lock()
				p.release()
				p.mu.Unlock()
				c = nil
			}
			return c, err
		}

		if !p.Wait {
			p.mu.Unlock()
			return nil, ErrConnPoolExhausted
		}

		if p.cond == nil {
			p.cond = sync.NewCond(&p.mu)
		}
		p.cond.Wait()
	}
}

func (p *ConnPool) Put(c *ThriftConn, forceClose bool) error {
	//err := c.Err()
	p.mu.Lock()
	//if !p.closed && err == nil && !forceClose {
	if !p.closed && !forceClose {
		p.idle.PushFront(idleConn{t: nowFunc(), c: c})
		if p.idle.Len() > p.MaxIdle {
			c = p.idle.Remove(p.idle.Back()).(idleConn).c
		} else {
			c = nil
		}
	}

	if c == nil {
		if p.cond != nil {
			p.cond.Signal()
		}
		p.mu.Unlock()
		return nil
	}

	p.release()
	p.mu.Unlock()
	return c.Close()
}

/*
type pooledConnection struct {
	p     *ConnPool
	c     Conn
	state int
}

type errorConnection struct{ err error }
*/

/* main.c  -  Foundation mutex test  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
 * 
 * This library provides a cross-platform foundation library in C11 providing basic support data types and
 * functions to write applications and games in a platform-independent fashion. The latest source code is
 * always available at
 * 
 * https://github.com/rampantpixels/foundation_lib
 * 
 * This library is put in the public domain; you can redistribute it and/or modify it without any restrictions.
 *
 */

#include <foundation/foundation.h>
#include <test/test.h>


static application_t test_mutex_application( void )
{
	application_t app = {0};
	app.name = "Foundation mutex tests";
	app.short_name = "test_mutex";
	app.config_dir = "test_mutex";
	app.flags = APPLICATION_UTILITY;
	app.dump_callback = test_crash_handler;
	return app;
}


static memory_system_t test_mutex_memory_system( void )
{
	return memory_system_malloc();
}


static int test_mutex_initialize( void )
{
	return 0;
}


static void test_mutex_shutdown( void )
{
}


DECLARE_TEST( mutex, basic )
{
	mutex_t* mutex;

	mutex = mutex_allocate( "test" );

	EXPECT_STREQ( mutex_name( mutex ), "test" );
	EXPECT_TRUE( mutex_try_lock( mutex ) );
	EXPECT_TRUE( mutex_lock( mutex ) );
	EXPECT_TRUE( mutex_try_lock( mutex ) );
	EXPECT_TRUE( mutex_lock( mutex ) );
	
	EXPECT_TRUE( mutex_unlock( mutex ) );
	EXPECT_TRUE( mutex_unlock( mutex ) );
	EXPECT_TRUE( mutex_unlock( mutex ) );
	EXPECT_TRUE( mutex_unlock( mutex ) );

	log_set_suppress( 0, ERRORLEVEL_WARNING );
	EXPECT_FALSE( mutex_unlock( mutex ) );
	log_set_suppress( 0, ERRORLEVEL_INFO );

	mutex_signal( mutex );
	thread_yield();
	EXPECT_TRUE( mutex_wait( mutex, 1 ) );
	EXPECT_TRUE( mutex_unlock( mutex ) );

	log_set_suppress( 0, ERRORLEVEL_WARNING );
	EXPECT_FALSE( mutex_wait( mutex, 100 ) );
	EXPECT_FALSE( mutex_unlock( mutex ) );
	log_set_suppress( 0, ERRORLEVEL_INFO );

	mutex_signal( mutex );
	thread_yield();
	EXPECT_TRUE( mutex_wait( mutex, 1 ) );
	log_set_suppress( 0, ERRORLEVEL_WARNING );
	EXPECT_FALSE( mutex_wait( mutex, 100 ) );
	EXPECT_TRUE( mutex_unlock( mutex ) );
	EXPECT_FALSE( mutex_unlock( mutex ) );
	log_set_suppress( 0, ERRORLEVEL_INFO );
	
	mutex_deallocate( mutex );
	
	return 0;
}


unsigned int thread_counter = 0;

static void* mutex_thread( object_t thread, void* arg )
{
	mutex_t* mutex = arg;
	int i;
	
	for( i = 0; i < 128; ++i )
	{
		if( !mutex_try_lock( mutex ) )
			mutex_lock( mutex );

		++thread_counter;

		mutex_unlock( mutex );
	}
	
	return 0;
}


DECLARE_TEST( mutex, sync )
{
	mutex_t* mutex;
	object_t thread[32];
	int ith;

	mutex = mutex_allocate( "test" );
	mutex_lock( mutex );
	
	for( ith = 0; ith < 32; ++ith )
	{
		thread[ith] = thread_create( mutex_thread, "mutex_thread", THREAD_PRIORITY_NORMAL, 0 );
		thread_start( thread[ith], mutex );
	}

	test_wait_for_threads_startup( thread, 32 );
	
	for( ith = 0; ith < 32; ++ith )
	{
		thread_terminate( thread[ith] );
		thread_destroy( thread[ith] );
	}

	mutex_unlock( mutex );

	test_wait_for_threads_exit( thread, 32 );
	
	mutex_deallocate( mutex );

	EXPECT_EQ( thread_counter, 32 * 128 );
	
	return 0;
}


atomic32_t thread_waiting = {0};
atomic32_t thread_waited = {0};


static void* thread_wait( object_t thread, void* arg )
{
	mutex_t* mutex = arg;

	atomic_incr32( &thread_waiting );

	if( mutex_wait( mutex, 30000 ) )
	{
		atomic_incr32( &thread_waited );
		mutex_unlock( mutex );
	}
	else
	{
		log_warn( HASH_TEST, WARNING_SUSPICIOUS, "Thread timeout" );
	}

	return 0;
}
	

DECLARE_TEST( mutex, signal )
{
	mutex_t* mutex;
	object_t thread[32];
	int ith;

	mutex = mutex_allocate( "test" );
	mutex_lock( mutex );
	
	for( ith = 0; ith < 32; ++ith )
	{
		thread[ith] = thread_create( thread_wait, "thread_wait", THREAD_PRIORITY_NORMAL, 0 );
		thread_start( thread[ith], mutex );
	}

	mutex_unlock( mutex );
	
	test_wait_for_threads_startup( thread, 32 );

	while( atomic_load32( &thread_waiting ) < 32 )
		thread_yield();
	thread_sleep( 1000 ); //Hack wait to give threads time to progress from atomic_incr to mutex_wait
	
	mutex_signal( mutex );

	for( ith = 0; ith < 32; ++ith )
	{
		thread_terminate( thread[ith] );
		thread_destroy( thread[ith] );
	}
	
	test_wait_for_threads_exit( thread, 32 );

	EXPECT_EQ( atomic_load32( &thread_waited ), 32 );

	EXPECT_FALSE( mutex_wait( mutex, 500 ) );
	
	mutex_deallocate( mutex );

	return 0;
}


static void test_mutex_declare( void )
{
	ADD_TEST( mutex, basic );
	ADD_TEST( mutex, sync );
	ADD_TEST( mutex, signal );
}


test_suite_t test_mutex_suite = {
	test_mutex_application,
	test_mutex_memory_system,
	test_mutex_declare,
	test_mutex_initialize,
	test_mutex_shutdown
};


#if FOUNDATION_PLATFORM_ANDROID || FOUNDATION_PLATFORM_IOS

int test_mutex_run( void );
int test_mutex_run( void )
{
	test_suite = test_mutex_suite;
	return test_run_all();
}

#else

test_suite_t test_suite_define( void );
test_suite_t test_suite_define( void )
{
	return test_mutex_suite;
}

#endif

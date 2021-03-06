/* main.c  -  Foundation stacktrace test  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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


static application_t test_stacktrace_application( void )
{
	application_t app = {0};
	app.name = "Foundation stacktrace tests";
	app.short_name = "test_stacktrace";
	app.config_dir = "test_stacktrace";
	app.flags = APPLICATION_UTILITY;
	app.dump_callback = test_crash_handler;
	return app;
}


static memory_system_t test_stacktrace_memory_system( void )
{
	return memory_system_malloc();
}


static int test_stacktrace_initialize( void )
{
	return 0;
}


static void test_stacktrace_shutdown( void )
{
}


DECLARE_TEST( stacktrace, capture )
{
	#define TEST_DEPTH 64
	void* trace[TEST_DEPTH];
	unsigned int num_frames = 0;

	num_frames = stacktrace_capture( trace, TEST_DEPTH, 0 );
	EXPECT_GT( num_frames, 3 );
	
	return 0;
}


DECLARE_TEST( stacktrace, resolve )
{
	#define TEST_DEPTH 64
	void* trace[TEST_DEPTH];
	unsigned int num_frames = 0;
	char* resolved = 0;

	num_frames = stacktrace_capture( trace, TEST_DEPTH, 0 );
	EXPECT_GT( num_frames, 3 );

	resolved = stacktrace_resolve( trace, num_frames, 0 );
	EXPECT_NE( resolved, 0 );

	log_infof( HASH_TEST, "Resolved stack trace:\n%s", resolved );
	
#if !FOUNDATION_PLATFORM_ANDROID
	EXPECT_NE( string_find_string( resolved, "stacktraceresolve_fn", 0 ), STRING_NPOS );
	EXPECT_NE( string_find_string( resolved, "test_run", 0 ), STRING_NPOS );
	//EXPECT_NE( string_find_string( resolved, "main_run", 0 ), STRING_NPOS );
#endif

	memory_deallocate( resolved );
	
	return 0;
}


static void test_stacktrace_declare( void )
{
	ADD_TEST( stacktrace, capture );
	ADD_TEST( stacktrace, resolve );
}


test_suite_t test_stacktrace_suite = {
	test_stacktrace_application,
	test_stacktrace_memory_system,
	test_stacktrace_declare,
	test_stacktrace_initialize,
	test_stacktrace_shutdown
};


#if FOUNDATION_PLATFORM_ANDROID || FOUNDATION_PLATFORM_IOS

int test_stacktrace_run( void );
int test_stacktrace_run( void )
{
	test_suite = test_stacktrace_suite;
	return test_run_all();
}

#else

test_suite_t test_suite_define( void );
test_suite_t test_suite_define( void )
{
	return test_stacktrace_suite;
}

#endif

/* main.c  -  Foundation test launcher  -  Public Domain  -  2013 Mattias Jansson / Rampant Pixels
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

#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID

volatile bool _test_should_start = false;
volatile bool _test_should_terminate = false;

#endif

static void* event_thread( object_t thread, void* arg )
{
	event_block_t* block;
	event_t* event = 0;
	
	while( !thread_should_terminate( thread ) )
	{
		block = event_stream_process( system_event_stream() );
		event = 0;
		
		while( ( event = event_next( block, event ) ) )
		{
			switch( event->id )
			{
				case FOUNDATIONEVENT_START:
#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID
					log_infof( HASH_TEST, "Application start event received" );
					_test_should_start = true;
#endif
					break;
					
				case FOUNDATIONEVENT_TERMINATE:
#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID
					log_infof( HASH_TEST, "Application terminate event received" );
					_test_should_terminate = true;
#else
					log_warn( HASH_TEST, WARNING_SUSPICIOUS, "Terminating tests due to event" );
					process_exit( -2 );
#endif
					break;
					
				default:
					break;
			}
		}
		
		thread_sleep( 10 );
	}
	
	return 0;
}


#if FOUNDATION_PLATFORM_IOS && BUILD_ENABLE_LOG

#include <foundation/delegate.h>
#include <test/test.h>

static void test_log_callback( uint64_t context, int severity, const char* msg )
{
	if( _test_should_terminate )
		return;
	test_text_view_append( delegate_uiwindow(), 1 , msg );
}

#endif


int main_initialize( void )
{
	application_t application = {0};
	application.name = "Foundation library test suite";
	application.short_name = "test_all";
	application.config_dir = "test_all";
	application.flags = APPLICATION_UTILITY;
	
	log_set_suppress( 0, ERRORLEVEL_DEBUG );
	
#if FOUNDATION_PLATFORM_IOS && BUILD_ENABLE_LOG
	log_set_callback( test_log_callback );
#endif
	
	return foundation_initialize( memory_system_malloc(), application );
}


#if FOUNDATION_PLATFORM_ANDROID
#  include <foundation/android.h>
#endif

#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID
//extern int test_app_run( void );
extern int test_array_run( void );
extern int test_atomic_run( void );
extern int test_base64_run( void );
extern int test_bitbuffer_run( void );
extern int test_blowfish_run( void );
extern int test_bufferstream_run( void );
extern int test_config_run( void );
extern int test_crash_run( void );
extern int test_environment_run( void );
extern int test_error_run( void );
extern int test_event_run( void );
extern int test_fs_run( void );
extern int test_hash_run( void );
extern int test_hashmap_run( void );
extern int test_hashtable_run( void );
extern int test_library_run( void );
extern int test_math_run( void );
extern int test_md5_run( void );
extern int test_mutex_run( void );
extern int test_objectmap_run( void );
extern int test_path_run( void );
extern int test_pipe_run( void );
extern int test_profile_run( void );
extern int test_radixsort_run( void );
extern int test_random_run( void );
extern int test_ringbuffer_run( void );
extern int test_semaphore_run( void );
extern int test_stacktrace_run( void );
extern int test_string_run( void );
extern int test_uuid_run( void );
typedef int (*test_run_fn)( void );

static void* test_runner( object_t obj, void* arg )
{
	test_run_fn* tests = (test_run_fn*)arg;
	int test_fn = 0;
	int process_result = 0;
	
	while( tests[test_fn] && ( process_result >= 0 ) )
	{
		if( ( process_result = tests[test_fn]() ) >= 0 )
			log_infof( HASH_TEST, "All tests passed (%d)", process_result );
		++test_fn;
	}
	
	return (void*)(intptr_t)process_result;
}

#endif


static const char* test_arch_name[11] = {
  "x86",
  "x86-64",
  "ppc",
  "ppc64",
  "arm5",
  "arm6",
  "arm7",
  "arm8",
  "arm8-64",
  "mips",
  "mips64"
};

int main_run( void* main_arg )
{
#if !FOUNDATION_PLATFORM_IOS && !FOUNDATION_PLATFORM_ANDROID
	const char* pattern = 0;
	char** exe_paths = 0;
	unsigned int iexe, exesize;
	process_t* process = 0;
	char* process_path = 0;
	unsigned int* exe_flags = 0;
#endif
#if BUILD_DEBUG
	const char* build_name = "debug";
#elif BUILD_RELEASE
	const char* build_name = "release";
#elif BUILD_PROFILE
	const char* ATTRIBUTE(unused) build_name = "profile";
#elif BUILD_DEPLOY
	const char* ATTRIBUTE(unused) build_name = "deploy";
#endif
	int process_result = 0;
	object_t thread = 0;
	
	log_set_suppress( HASH_TEST, ERRORLEVEL_DEBUG );

	log_infof( HASH_TEST, "Foundation library v%s built for %s (%s)", string_from_version_static( foundation_version() ), test_arch_name[ system_architecture() ], build_name );
	FOUNDATION_UNUSED( test_arch_name );
	
	thread = thread_create( event_thread, "event_thread", THREAD_PRIORITY_NORMAL, 0 );
	thread_start( thread, 0 );
	while( !thread_is_running( thread ) )
		thread_sleep( 10 );
	
#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID
	while( !_test_should_start )
	{
#if FOUNDATION_PLATFORM_ANDROID
		system_process_events();
#endif
		thread_sleep( 100 );
	}
#endif

	fs_remove_directory( environment_temporary_directory() );
	
#if FOUNDATION_PLATFORM_IOS || FOUNDATION_PLATFORM_ANDROID
	
	test_run_fn tests[] = {
		//test_app_run
		test_array_run,
		test_atomic_run,
		test_base64_run,
		test_bitbuffer_run,
		test_blowfish_run,
		test_bufferstream_run,
		test_config_run,
		test_crash_run,
		test_environment_run,
		test_error_run,
		test_event_run,
		test_fs_run,
		test_hash_run,
		test_hashmap_run,
		test_hashtable_run,
		test_library_run,
		test_math_run,
		test_md5_run,
		test_mutex_run,
		test_objectmap_run,
		test_path_run,
		test_pipe_run,
		test_profile_run,
		test_radixsort_run,
		test_random_run,
		test_ringbuffer_run,
		test_semaphore_run,
		test_stacktrace_run,
		test_string_run,
		test_uuid_run,
		0
	};
	
#if FOUNDATION_PLATFORM_ANDROID
	
	object_t test_thread = thread_create( test_runner, "test_runner", THREAD_PRIORITY_NORMAL, 0 );
	thread_start( test_thread, tests );

	log_infof( HASH_TEST, "Starting test runner thread" );

	while( !thread_is_running( test_thread ) )
	{
		system_process_events();
		thread_sleep( 10 );
	}
	
	while( thread_is_running( test_thread ) )
	{
		system_process_events();
		thread_sleep( 10 );
	}
	
	process_result = (int)(intptr_t)thread_result( test_thread );
	thread_destroy( test_thread );
	
	while( thread_is_thread( test_thread ) )
	{
		system_process_events();
		thread_sleep( 10 );
	}
	
#else
	
	process_result = (int)(intptr_t)test_runner( 0, tests );
	
#endif
	
	if( process_result != 0 )
		log_warnf( HASH_TEST, WARNING_SUSPICIOUS, "Tests failed with exit code %d", process_result );
	
	while( !_test_should_terminate )
	{
		system_process_events();
		thread_sleep( 100 );
	}
	
	log_debug( HASH_TEST, "Exiting main loop" );
	
#else
	
	//Find all test executables in the current executable directory
#if FOUNDATION_PLATFORM_WINDOWS
	pattern = "^test-.*.exe$";
#elif FOUNDATION_PLATFORM_MACOSX
	pattern = "^test-.*$";
#elif FOUNDATION_PLATFORM_POSIX
	pattern = "^test-.*$";
#else
#  error Not implemented
#endif
	exe_paths = fs_matching_files( environment_executable_directory(), pattern, false );
	array_resize( exe_flags, array_size( exe_paths ) );
	memset( exe_flags, 0, sizeof( unsigned int ) * array_size( exe_flags ) );
#if FOUNDATION_PLATFORM_MACOSX
	//Also search for test applications
	const char* app_pattern = "^test-.*\\.app$";
	char** subdirs = fs_subdirs( environment_executable_directory() );
	for( int idir = 0, dirsize = array_size( subdirs ); idir < dirsize; ++idir )
	{
		if( string_match_pattern( subdirs[idir], app_pattern ) )
		{
			array_push( exe_paths, string_substr( subdirs[idir], 0, string_length( subdirs[idir] ) - 4 ) );
			array_push( exe_flags, PROCESS_OSX_USE_OPENAPPLICATION );
		}
	}
	string_array_deallocate( subdirs );
#endif
	for( iexe = 0, exesize = array_size( exe_paths ); iexe < exesize; ++iexe )
	{
		bool is_self = false;
		char* exe_file_name = path_base_file_name( exe_paths[iexe] );
		if( string_equal( exe_file_name, environment_executable_name() ) )
			is_self = true;
		string_deallocate( exe_file_name );
		if( is_self )
			continue; //Don't run self
		
		process_path = path_merge( environment_executable_directory(), exe_paths[iexe] );
		
		process = process_allocate();
		
		process_set_executable_path( process, process_path );
		process_set_working_directory( process, environment_executable_directory() );
		process_set_flags( process, PROCESS_ATTACHED | exe_flags[iexe] );
		
		log_infof( HASH_TEST, "Running test executable: %s", exe_paths[iexe] );
		
		process_result = process_spawn( process );
		while( process_result == PROCESS_WAIT_INTERRUPTED )
		{
			thread_sleep( 10 );
			process_result = process_wait( process );
		}
		process_deallocate( process );
		
		string_deallocate( process_path );
		
		if( process_result != 0 )
		{
			if( process_result >= PROCESS_INVALID_ARGS )
				log_warnf( HASH_TEST, WARNING_SUSPICIOUS, "Tests failed, process terminated with error %x", process_result );
			else
				log_warnf( HASH_TEST, WARNING_SUSPICIOUS, "Tests failed with exit code %d", process_result );
			process_set_exit_code( -1 );
			goto exit;
		}
		
		log_infof( HASH_TEST, "All tests from %s passed (%d)", exe_paths[iexe], process_result );
	}
	
	log_info( HASH_TEST, "All tests passed" );
	
exit:
	
	if( exe_paths )
		string_array_deallocate( exe_paths );
	array_deallocate( exe_flags );
	
#endif
	
	thread_terminate( thread );
	thread_destroy( thread );
	while( thread_is_running( thread ) )
		thread_sleep( 10 );
	while( thread_is_thread( thread ) )
		thread_sleep( 10 );
	
	return process_result;
}


void main_shutdown( void )
{
	foundation_shutdown();
}


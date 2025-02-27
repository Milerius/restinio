#include <iostream>

#include <restinio/all.hpp>

// Class for connection state listener.
class connection_listener_t
{
	// Because method of connection_listener_t can be called from
	// different threads at the same time we have to protect connection
	// listener.
	std::mutex m_lock;

	static const char *
	cause_to_str( restinio::connection_state::cause_t cause ) noexcept
	{
		const char * result = "unknown";
		switch( cause )
		{
			case restinio::connection_state::cause_t::accepted:
				result = "accepted";
			break;

			case restinio::connection_state::cause_t::closed:
				result = "closed";
			break;

			case restinio::connection_state::cause_t::upgraded_to_websocket:
				result = "upgraded";
			break;
		}
		return result;
	}

public:
	// This method will be called by RESTinio.
	void state_changed(
		const restinio::connection_state::notice_t & notice ) noexcept
	{
		std::lock_guard<std::mutex> l{ m_lock };

		fmt::print( "connection-notice: {} (from {}) => {}\n",
				notice.connection_id(),
				notice.remote_endpoint(),
				cause_to_str( notice.cause() ) );
	}
};

// This is the request handler.
restinio::request_handling_status_t handler( restinio::request_handle_t req )
{
	if( restinio::http_method_get() == req->header().method() &&
		req->header().request_target() == "/" )
	{
		req->create_response()
			.append_header( restinio::http_field::server, "RESTinio hello world server" )
			.append_header_date_field()
			.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
			.set_body( fmt::format( "{} (from {}): Hello world!",
					req->connection_id(),
					req->remote_endpoint() ) )
			.done();

		return restinio::request_accepted();
	}

	return restinio::request_rejected();
}

int main()
{
	struct my_traits_t : public restinio::default_traits_t
	{
		// Change the default connection state listener to our one.
		using connection_state_listener_t = connection_listener_t;
	};

	try
	{
		restinio::run(
			restinio::on_thread_pool< my_traits_t >( std::thread::hardware_concurrency() )
				.port( 8080 )
				.address( "localhost" )
				// Connection listener must be set manually.
				.connection_state_listener(
						std::make_shared< connection_listener_t >() )
				.request_handler( handler ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

#pragma once

namespace fast_io
{

namespace details
{
template<std::size_t mx_size,::fast_io::freestanding::random_access_iterator Iter>
inline constexpr Iter output_unsigned_serialize_size(std::size_t val,Iter iter) noexcept;

template<::fast_io::freestanding::random_access_iterator Iter,my_unsigned_integral U>
inline constexpr void output_unsigned_with_size(Iter str,U value,std::size_t len) noexcept;
}


namespace details::decay
{

#if 0
enum class print_control_impl
{
normal,serialize
};


inline constexpr std::size_t reserve_len(std::size_t sz) noexcept
{
	std::size_t len{};
	for(;sz;sz/=10)
		++len;
	return len;
}

template<print_control_impl pci,std::integral char_type,typename value_type>
requires reserve_printable<char_type,value_type>
inline constexpr std::size_t print_reserve_control_size_impl() noexcept
{
	constexpr std::size_t sz{print_reserve_size(io_reserve_type<char_type,value_type>)};
	if constexpr(pci==print_control_impl::serialize)
	{
		constexpr std::size_t len(reserve_len(sz)+sz+1);
		return len;
	}
	else
		return sz;
}

template<print_control_impl pci,std::integral char_type,typename value_type>
requires dynamic_reserve_printable<char_type,value_type>
inline constexpr std::size_t print_dynamic_reserve_control_size_impl(value_type v)
{
	std::size_t sz{print_reserve_size(io_reserve_type<char_type,value_type>,v)};
	if constexpr(pci==print_control_impl::serialize)
	{
		std::size_t len(reserve_len(sz)+sz+1);
		return len;
	}
	else
		return sz;
}

template<print_control_impl pci,std::integral char_type,typename value_type,typename Iter>
inline constexpr Iter print_reserve_control_define_impl(Iter it,value_type v)
{
	if constexpr(pci==print_control_impl::serialize)
	{
		if constexpr(reserve_serializable<char_type,value_type>)
			return print_reserve_define(io_serial_type<char_type,value_type>,it,v);
		else
		{
			constexpr std::size_t sz{print_reserve_size(io_reserve_type<char_type,value_type>)};
			constexpr std::size_t reserved_sz{reserve_len(sz)};
			constexpr std::size_t reservedln{reserved_sz+1};
			auto start{it+reservedln};
			auto eds{print_reserve_define(io_reserve_type<char_type,value_type>,start,v)};
			std::size_t real_size(eds-start);
			std::size_t real_size_size{chars_len<10,false,reserved_sz>(real_size)};
			output_unsigned_with_size(it,real_size,real_size_size);
			if constexpr(std::same_as<char,char_type>)
				it[real_size_size]=' ';
			else if constexpr(std::same_as<wchar_t,char_type>)
				it[real_size_size]=L' ';
			else
				it[real_size_size]=u8' ';
			auto realszp1{it+real_size_size+1};
			if(start==realszp1)
				return eds;
			return my_copy_n(start,real_size,realszp1);
		}
	}
	else
		return print_reserve_define(io_reserve_type<char_type,value_type>,it,v);
}


template<std::integral char_type,print_control_impl pci=print_control_impl::normal,typename T>
inline constexpr void scatter_print_recursive(io_scatter_t* arr,T t)
{
	*arr=print_scatter_define(print_scatter_type<char_type>,t);
}

template<std::integral char_type,print_control_impl pci=print_control_impl::normal,typename T,typename... Args>
inline constexpr void scatter_print_recursive(io_scatter_t* arr,T t, Args ...args)
{
	*arr=print_scatter_define(print_scatter_type<char_type>,t);
	scatter_print_recursive<char_type>(arr+1,args...);
}
#else
template<std::integral char_type,typename T>
inline constexpr void scatter_print_recursive(io_scatter_t* arr,T t)
{
	*arr=print_scatter_define(print_scatter_type<char_type>,t);
}

template<std::integral char_type,typename T,typename... Args>
inline constexpr void scatter_print_recursive(io_scatter_t* arr,T t, Args ...args)
{
	*arr=print_scatter_define(print_scatter_type<char_type>,t);
	scatter_print_recursive<char_type>(arr+1,args...);
}
#endif

template<std::integral char_type,typename T>
inline constexpr std::size_t calculate_scatter_reserve_size_unit()
{
	using real_type = std::remove_cvref_t<T>;
	if constexpr(reserve_printable<char_type,real_type>)
	{
		constexpr std::size_t sz{print_reserve_size(io_reserve_type<char_type,real_type>)};
		return sz;
	}
	else
		return 0;
}

template<std::integral char_type,typename T>
inline constexpr std::size_t calculate_scatter_reserve_size()
{
	return calculate_scatter_reserve_size_unit<char_type,T>();
}

template<std::integral char_type,typename T,typename... Args>
requires (sizeof...(Args)!=0)
inline constexpr std::size_t calculate_scatter_reserve_size()
{
	return ::fast_io::details::intrinsics::add_or_overflow_die(calculate_scatter_reserve_size_unit<char_type,T>(),
		calculate_scatter_reserve_size<char_type,Args...>());
}



template<std::integral char_type,typename T,typename... Args>
inline constexpr std::size_t calculate_scatter_dynamic_reserve_size([[maybe_unused]] T t,Args... args)
{
	if constexpr(!reserve_printable<char_type,T>&&
		dynamic_reserve_printable<char_type,T>)
	{
		std::size_t res{print_reserve_size(io_reserve_type<char_type,T>,t)};
		if constexpr(sizeof...(Args)==0)
			return res;
		else
			return ::fast_io::details::intrinsics::add_or_overflow_die(res,calculate_scatter_dynamic_reserve_size<char_type>(args...));
	}
	else
	{
		if constexpr(sizeof...(Args)==0)
			return 0;
		else
			return calculate_scatter_dynamic_reserve_size<char_type>(args...);
	}
}


template<std::integral char_type,typename T>
inline constexpr void scatter_print_with_reserve_recursive_unit(char_type*& start_ptr,
		io_scatter_t* arr,T t)
{
	using real_type = std::remove_cvref_t<T>;
	if constexpr(reserve_printable<char_type,T>)
	{
#ifdef __SANITIZE_ADDRESS__
		constexpr std::size_t sz{print_reserve_size(io_reserve_type<char_type,real_type>)};
		char_type sanitize_buffer[sz];
		auto dt{print_reserve_define(io_reserve_type<char_type,real_type>,sanitize_buffer,t)};
		auto end_ptr{non_overlapped_copy_n(sanitize_buffer,static_cast<std::size_t>(dt-sanitize_buffer),start_ptr)};
		*arr={start_ptr,(end_ptr-start_ptr)*sizeof(*start_ptr)};
		start_ptr=end_ptr;
#else
		auto end_ptr = print_reserve_define(io_reserve_type<char_type,real_type>,start_ptr,t);
		*arr={start_ptr,(end_ptr-start_ptr)*sizeof(*start_ptr)};
		start_ptr=end_ptr;
#endif
	}
	else
	{
		*arr=print_scatter_define(print_scatter_type<char_type>,t);
	}
}

template<std::integral char_type,typename T>
inline constexpr void scatter_print_with_reserve_recursive(char_type* ptr,
		io_scatter_t* arr,T t)
{
	scatter_print_with_reserve_recursive_unit(ptr,arr,t);
}

template<std::integral char_type,typename T,typename... Args>
inline constexpr void scatter_print_with_reserve_recursive(char_type* ptr,
	io_scatter_t* arr,T t, Args ...args)
{
	scatter_print_with_reserve_recursive_unit(ptr,arr,t);
	scatter_print_with_reserve_recursive(ptr,arr+1,args...);
}

template<std::integral char_type,typename T,typename... Args>
inline constexpr void scatter_print_with_dynamic_reserve_recursive(io_scatter_t* __restrict arr,
	[[maybe_unused]] char_type* __restrict ptr,
	[[maybe_unused]] char_type* __restrict dynamic_buffer_ptr,[[maybe_unused]] T t, Args ...args)
{
	
	if constexpr(scatter_type_printable<char_type,T>)
		*arr=print_scatter_define(print_scatter_type<char_type>,t);
	else if constexpr(reserve_printable<char_type,T>)
	{
		auto end_ptr = print_reserve_define(io_reserve_type<char_type,T>,ptr,t);
		*arr={ptr,(end_ptr-ptr)*sizeof(*ptr)};
		if constexpr(sizeof...(Args)!=0)
			ptr=end_ptr;
	}
	else if constexpr(dynamic_reserve_printable<char_type,T>)
	{
		auto end_ptr = print_reserve_define(io_reserve_type<char_type,T>,dynamic_buffer_ptr,t);
		*arr={dynamic_buffer_ptr,(end_ptr-dynamic_buffer_ptr)*sizeof(*dynamic_buffer_ptr)};
		if constexpr(sizeof...(Args)!=0)
			dynamic_buffer_ptr = end_ptr;
	}
	if constexpr(sizeof...(Args)!=0)
		scatter_print_with_dynamic_reserve_recursive(arr+1,ptr,dynamic_buffer_ptr,args...);
}


template<std::integral char_type,typename T>
requires scatter_type_printable<char_type,T>
inline constexpr auto extract_one_scatter(T t)
{
	return print_scatter_define(print_scatter_type<char_type>,t);
}

template<std::integral char_type>
inline constexpr char_type print_reserve_get_lf() noexcept
{
	if constexpr(std::same_as<char,char_type>)
		return '\n';
	else if constexpr(std::same_as<wchar_t,char_type>)
		return L'\n';
	else
		return u8'\n';
}

template<std::integral char_type>
inline constexpr auto lf_value{print_reserve_get_lf<char_type>()};

template<typename output>
inline constexpr void print_serialize_size_bad_path(output out,std::size_t sz)
{
	using char_type = typename output::char_type;
	constexpr std::size_t size_t_reserve_length{print_reserve_size(io_reserve_type<char_type,std::size_t>)+1};
	char_type array[size_t_reserve_length];
	auto it{print_reserve_define(io_reserve_type<char_type,std::size_t>,array,sz)};
	*it=lf_value<char_type>;
	++it;
	write(out,array,it);
}

template<bool line,
#if 0
print_control_impl pci,
#endif
typename output,typename value_type>
#if __has_cpp_attribute(gnu::cold)
[[gnu::cold]]
#endif
inline constexpr void print_control_reserve_bad_path(output out,value_type t)
{
	using char_type = typename output::char_type;
#if 0
	constexpr std::size_t size{print_reserve_control_size_impl<pci,char_type,value_type>()+static_cast<std::size_t>(line)};
#else
	constexpr std::size_t size{print_reserve_size(io_reserve_type<char_type,value_type>)+static_cast<std::size_t>(line)};
#endif
	char_type array[size];
	if constexpr(line)
	{
#if 0
		auto it{print_reserve_control_define_impl<pci,char_type,value_type>(array,t)};
#else
		auto it{print_reserve_define(io_reserve_type<char_type,value_type>,array,t)};
#endif
		*it=lf_value<char_type>;
		++it;
		write(out,array,it);
	}
	else
#if 0
		write(out,array,print_reserve_control_define_impl<pci,char_type,value_type>(array,t));
#else
		write(out,array,print_reserve_define(io_reserve_type<char_type,value_type>,array,t));
#endif
}

template<bool line,typename value_type,output_stream output>
inline constexpr void print_control_dynamic_reserve_bad_path(output out,value_type t,std::size_t sizep1)
{
	using char_type = typename output::char_type;
	local_operator_new_array_ptr<char_type> ptr(sizep1);
#if 0
	auto it{print_reserve_control_define_impl<pci,char_type,value_type>(ptr.ptr,t)};
#else
	auto it{print_reserve_define(io_reserve_type<char_type,value_type>,ptr.ptr,t)};
#endif
	if constexpr(line)
	{
		*it=lf_value<char_type>;
		++it;
	}
	write(out,ptr.ptr,it);
}

template<bool line=false,
#if 0
print_control_impl pci=print_control_impl::normal,
#endif
output_stream output,typename T>
requires (std::is_trivially_copyable_v<output>&&std::is_trivially_copyable_v<T>)
inline constexpr void print_control(output out,T t)
{
	using char_type = typename output::char_type;
	using value_type = std::remove_cvref_t<T>;
	constexpr auto lfch{lf_value<char_type>};
	if constexpr(scatter_type_printable<char_type,value_type>)
	{
		basic_io_scatter_t<char_type> scatter{print_scatter_define(print_scatter_type<char_type>,t)};
		if constexpr(line)
		{
			if constexpr(contiguous_output_stream<output>)
			{
				auto curr=obuffer_curr(out);
				auto end=obuffer_end(out);
				std::ptrdiff_t sz(end-curr-1);
				std::size_t const len{scatter.len};
				if(sz<static_cast<std::ptrdiff_t>(len))
					fast_terminate();
				curr=non_overlapped_copy_n(scatter.base,scatter.len,curr);
				*curr=lfch;
				++curr;
				obuffer_set_curr(out,curr);
			}
			else if constexpr(buffer_output_stream<output>)
			{
				auto curr=obuffer_curr(out);
				auto end=obuffer_end(out);
				std::size_t const len{scatter.len};
				std::ptrdiff_t sz(end-curr-1);
#if 0
				constexpr std::size_t size_t_reserve_length{print_reserve_size(io_reserve_type<char_type,std::size_t>)+1};
				if constexpr(pci==print_control_impl::serialize)
				{
					sz-=size_t_reserve_length;
				}
#endif
				if(static_cast<std::ptrdiff_t>(len)<sz)[[likely]]
				{
#if 0
					if constexpr(pci==print_control_impl::serialize)
					{
					curr=print_reserve_define(io_reserve_type<char_type,std::size_t>,curr,len);
					put(out,lfch);
					++curr;
					}
#endif
					curr=details::non_overlapped_copy_n(scatter.base,len,curr);
					*curr=lfch;
					++curr;
					obuffer_set_curr(out,curr);
				}
				else
				{
#if 0
					if constexpr(pci==print_control_impl::serialize)
					{
					print_serialize_size_bad_path(out,scatter.len);
					}
#endif
					write(out,scatter.base,scatter.base+scatter.len);
					put(out,lfch);
				}
			}
			else
			{
#if 0
				if constexpr(pci==print_control_impl::serialize)
					print_serialize_size_bad_path(out,scatter.len);
#endif
				write(out,scatter.base,scatter.base+scatter.len);
				put(out,lfch);
			}
		}
		else
		{
#if 0
			if constexpr(pci==print_control_impl::serialize)
				print_serialize_size_bad_path(out,scatter.len);
#endif
			write(out,scatter.base,scatter.base+scatter.len);
		}
	}
	else if constexpr(reserve_printable<char_type,value_type>)
	{
		constexpr std::size_t real_size{
#if 0
print_reserve_control_size_impl<pci,char_type,value_type>()
#else
print_reserve_size(io_reserve_type<char_type,value_type>)
#endif
};
		constexpr std::size_t size{real_size+static_cast<std::size_t>(line)};
		static_assert(real_size!=SIZE_MAX);

		if constexpr(contiguous_output_stream<output>)
		{
			auto bcurr{obuffer_curr(out)};
			auto bend{obuffer_end(out)};
			std::size_t diff{static_cast<std::size_t>(bend-bcurr)};
			if(diff<size)[[unlikely]]
				fast_terminate();
#if 0
			auto it{print_reserve_control_define_impl<pci,char_type,value_type>(bcurr,t)};
#else
			auto it{print_reserve_define(io_reserve_type<char_type,value_type>,bcurr,t)};
#endif
			if constexpr(line)
			{
				*it=lfch;
				++it;
			}
			obuffer_set_curr(out,it);
		}
#ifndef __SANITIZE_ADDRESS__
		else if constexpr(buffer_output_stream<output>)
		{
			auto bcurr{obuffer_curr(out)};
			auto bend{obuffer_end(out)};
			std::ptrdiff_t const diff(bend-bcurr);
			if(static_cast<std::ptrdiff_t>(size)<diff)[[likely]]
			{
				//To check whether this affects performance.
				if constexpr(line)
				{
#if 0
					auto it{print_reserve_control_define_impl<pci,char_type,value_type>(bcurr,t)};
#else
					auto it{print_reserve_define(io_reserve_type<char_type,value_type>,bcurr,t)};
#endif
					if constexpr(line)
					{
						*it=lfch;
						++it;
					}	
					obuffer_set_curr(out,it);
				}
				else
#if 0
					obuffer_set_curr(out,print_reserve_control_define_impl<pci,char_type,value_type>(bcurr,t));
#else
					obuffer_set_curr(out,print_reserve_define(io_reserve_type<char_type,value_type>,bcurr,t));
#endif
			}
			else
				print_control_reserve_bad_path<line>(out,t);
		}
#endif
		else
		{
			print_control_reserve_bad_path<line>(out,t);
		}
	}
	else if constexpr(dynamic_reserve_printable<char_type,value_type>)
	{
#if 0
		std::size_t size{print_dynamic_reserve_control_size_impl<pci,char_type,value_type>(t)};
#else
		std::size_t size{print_reserve_size(io_reserve_type<char_type,value_type>,t)};
#endif
		if constexpr(line)
		{
			constexpr std::size_t mx{std::numeric_limits<std::ptrdiff_t>::max()-1};
			if(size>=mx)
				fast_terminate();
			++size;
		}
		else
		{
			constexpr std::size_t mx{std::numeric_limits<std::ptrdiff_t>::max()};
			if(mx<size)
				fast_terminate();
		}
		if constexpr(contiguous_output_stream<output>)
		{
			auto bcurr{obuffer_curr(out)};
			auto bend{obuffer_end(out)};
#if 0
			auto it{dynamic_print_reserve_control_define_impl<pci,char_type,value_type>(bcurr,t,size)};
#else
			auto it{print_reserve_define(io_reserve_type<char_type,value_type>,bcurr,t,size)};
#endif
			std::size_t diff{static_cast<std::size_t>(bend-bcurr)};
			if(diff<size)[[unlikely]]
				fast_terminate();
			if constexpr(line)
			{
				*it=lfch;
				++it;
			}
			obuffer_set_curr(out,it);
		}
#ifndef __SANITIZE_ADDRESS__
		else if constexpr(buffer_output_stream<output>)
		{
			auto curr{obuffer_curr(out)};
			auto ed{obuffer_end(out)};
			std::ptrdiff_t diff(ed-curr);
			if(static_cast<std::ptrdiff_t>(size)<diff)
			{
#if 0
				auto it{print_reserve_control_define_impl<pci,char_type,value_type>(curr,t)};
#else
				auto it{print_reserve_define(io_reserve_type<char_type,value_type>,curr,t)};
#endif
				if constexpr(line)
				{
					*it=lfch;
					++it;
				}
				obuffer_set_curr(out,it);
			}
			else
#if 0
				print_control_dynamic_reserve_bad_path<line,pci,value_type>(out,t,size);
#else
				print_control_dynamic_reserve_bad_path<line,value_type>(out,t,size);
#endif
		}
#endif
		else
		{
#if 0
			print_control_dynamic_reserve_bad_path<line,pci,value_type>(out,t,size);
#else
			print_control_dynamic_reserve_bad_path<line,value_type>(out,t,size);
#endif
		}
	}
	else if constexpr(printable<output,value_type>)
	{
#if 0
		if constexpr(pci==print_control_impl::serialize)
		{
			dynamic_io_buffer<char_type> buffer;
			print_control(io_ref(buffer),t);
			auto beg{obuffer_begin(buffer)};
			auto curr{obuffer_curr(buffer)};
			std::size_t size(curr-beg);
			print_control(out,size);
			write(out,beg,curr);
		}
		else
#endif
			print_define(out,t);
		if constexpr(line)
		{
			put(out,lfch);
		}
	}
	else
	{
		constexpr bool no{printable<output,value_type>};
		static_assert(no,"type not printable");
	}
}

template<bool ln,output_stream output,typename T,typename... Args>
inline constexpr void print_controls_line(output out,T t,Args... args)
{
	if constexpr(sizeof...(Args)==0)
	{
		print_control<ln>(out,t);
	}
	else
	{
		print_control<false>(out,t);
		print_controls_line<ln>(out,args...);
	}
}
template<bool line,output_stream output,typename ...Args>
inline constexpr void print_fallback(output out,Args ...args)
{
	using char_type = typename output::char_type;
	constexpr auto lfch{lf_value<char_type>};
	if constexpr(line&&sizeof...(Args)==0)
	{
		char_type ch{lfch};
		write(out,__builtin_addressof(ch),__builtin_addressof(ch)+1);
	}
	else if constexpr((scatter_output_stream<output>&&((scatter_printable<typename output::char_type,Args>||reserve_printable<typename output::char_type,Args>||dynamic_reserve_printable<typename output::char_type,Args>)&&...)))
	{
		constexpr std::size_t n{(sizeof...(Args))+static_cast<std::size_t>(line)};
		io_scatter_t scatters[n];
		if constexpr((scatter_printable<typename output::char_type,Args>&&...))
		{
			scatter_print_recursive<typename output::char_type>(scatters,args...);
			if constexpr(line)
			{
				char_type ch{lfch};
				scatters[n-1]={__builtin_addressof(ch),sizeof(ch)};
				scatter_write(out,{scatters,n});
			}
			else
				scatter_write(out,{scatters,n});
		}
		else if constexpr(((scatter_printable<char_type,Args>||
			reserve_printable<char_type,Args>)&&...))
		{
			constexpr std::size_t total_size{calculate_scatter_reserve_size<char_type,Args...>()};
			char_type buffer[total_size];
			scatter_print_with_reserve_recursive(buffer,scatters,args...);
			if constexpr(line)
			{
				char_type ch{lfch};
				scatters[n-1]={__builtin_addressof(ch),sizeof(ch)};
				scatter_write(out,{scatters,n});
			}
			else
				scatter_write(out,{scatters,n});
		}
		else
		{
			constexpr std::size_t total_size{calculate_scatter_reserve_size<char_type,Args...>()};
			char_type buffer[total_size];
			local_operator_new_array_ptr<char_type> new_ptr(calculate_scatter_dynamic_reserve_size<char_type>(args...));
			scatter_print_with_dynamic_reserve_recursive(scatters,buffer,new_ptr.ptr,args...);
			if constexpr(line)
			{
				char_type ch{lfch};
				scatters[n-1]={__builtin_addressof(ch),sizeof(ch)};
				scatter_write(out,{scatters,n});
			}
			else
				scatter_write(out,{scatters,n});
		}
	}
	else
	{
		dynamic_io_buffer<typename output::char_type> buffer;
		auto ref{io_ref(buffer)};
		print_controls_line<line>(ref,args...);
		write(out,buffer.buffer_begin,buffer.buffer_curr);
	}
}

template<bool line,
#if 0
print_control_impl pci=print_control_impl::normal,
#endif
output_stream output,typename ...Args>
inline constexpr void print_freestanding_decay_normal(output out,Args ...args)
{
	using char_type = typename output::char_type;
	if constexpr(sizeof...(Args)==0&&!line)
		return;
	else if constexpr(mutex_stream<output>)
	{
		lock_guard lg{out};
		decltype(auto) dout{out.unlocked_handle()};
		print_freestanding_decay_normal<line>(io_ref(dout),args...);
	}
	else if constexpr(buffer_output_stream<output>)
	{
		if constexpr(sizeof...(Args)==0&&line)
			put(out,lf_value<char_type>);
		else
		{
			if constexpr(line)
				print_controls_line<line>(out,args...);
			else
				(print_control<line>(out,args),...);
		}
	}
	else if constexpr(sizeof...(Args)==1&&
		(((!line&&((printable<output,Args>||scatter_type_printable<char_type,Args>)&&...))||
		((reserve_printable<char_type,Args>||dynamic_reserve_printable<char_type,Args>)&&...))))
	{
		(print_control<line>(out,args),...);
	}
	else
	{
		print_fallback<line>(out,args...);
	}
}

}

template<typename output,typename ...Args>
requires (output_stream<output>||status_output_stream<output>)&&(std::is_trivially_copyable_v<output>&&(std::is_trivially_copyable_v<Args>&&...))
inline constexpr void print_freestanding_decay(output out,Args ...args)
{
	if constexpr(status_output_stream<output>)
		print_status_define(out,args...);
	else if constexpr(output_stream<output>)
		details::decay::print_freestanding_decay_normal<false>(out,args...);
}

template<typename output,typename ...Args>
requires (output_stream<output>||status_output_stream<output>)
inline constexpr void print_freestanding(output&& out,Args&& ...args)
{
	print_freestanding_decay(io_ref(out),io_print_forward<typename std::remove_cvref_t<output>::char_type>(io_print_alias(args))...);
}

namespace details
{

template<bool line,typename output,typename ...Args>
requires (output_stream<output>||status_output_stream<output>)
inline constexpr void print_freestanding_decay_cold_impl(output out,Args ...args)
{
	if constexpr(status_output_stream<output>)
	{
		if constexpr(line)
			println_status_define(out,args...);
		else
			print_status_define(out,args...);
	}
	else
		details::decay::print_freestanding_decay_normal<line>(out,args...);
}

}


template<typename output,typename ...Args>
requires (output_stream<output>||status_output_stream<output>)
inline constexpr void println_freestanding_decay(output out,Args ...args)
{
	if constexpr(status_output_stream<output>)
	{
		println_status_define(out,args...);
	}
	else
		details::decay::print_freestanding_decay_normal<true>(out,args...);
}

template<output_stream output,typename ...Args>
inline constexpr void println_freestanding(output&& out,Args&& ...args)
{
	println_freestanding_decay(io_ref(out),io_print_forward<typename std::remove_cvref_t<output>::char_type>(io_print_alias(args))...);
}


}
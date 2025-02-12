#pragma once
#if __has_include(<cstdio>)
#include<cstdio>
#elif __has_include(<stdio.h>)
#include<stdio.h>
#endif

#if defined(__MINGW32__) && !defined(_UCRT)
#include"msvcrt_lock.h"
#endif

namespace fast_io
{
namespace win32
{
#if defined(__MINGW32__) && !defined(_UCRT)
[[gnu::dllimport]] extern void __cdecl _lock_file(FILE*) noexcept asm("_lock_file");
[[gnu::dllimport]] extern void __cdecl _unlock_file(FILE*) noexcept asm("_unlock_file");
[[gnu::dllimport]] extern std::size_t __cdecl _fwrite_nolock(void const* __restrict buffer,std::size_t size,std::size_t count,FILE* __restrict) noexcept asm("_fwrite_nolock");
[[gnu::dllimport]] extern std::size_t __cdecl _fread_nolock(void* __restrict buffer,std::size_t size,std::size_t count,FILE* __restrict) noexcept asm("_fread_nolock");
[[gnu::dllimport]] extern std::size_t __cdecl fwrite(void const* __restrict buffer,std::size_t size,std::size_t count,FILE* __restrict) noexcept asm("fwrite");
[[gnu::dllimport]] extern std::size_t __cdecl fread(void* __restrict buffer,std::size_t size,std::size_t count,FILE* __restrict) noexcept asm("fread");
#endif
}

inline constexpr open_mode native_c_supported(open_mode m) noexcept
{
#ifdef _WIN32
using utype = typename std::underlying_type<open_mode>::type;
constexpr auto c_supported_values{static_cast<utype>(open_mode::text)|
	static_cast<utype>(open_mode::out)|
	static_cast<utype>(open_mode::app)|
	static_cast<utype>(open_mode::in)|
	static_cast<utype>(open_mode::trunc)};
return static_cast<open_mode>(static_cast<utype>(m)&c_supported_values);
#else
return c_supported(m);
#endif
}
inline constexpr char const* to_native_c_mode(open_mode m) noexcept
{
#ifdef _WIN32
/*
https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?view=vs-2019
From microsoft's document. _fdopen only supports

"r"	Opens for reading. If the file does not exist or cannot be found, the fopen call fails.
"w"	Opens an empty file for writing. If the given file exists, its contents are destroyed.
"a"	Opens for writing at the end of the file (appending). Creates the file if it does not exist.
"r+"	Opens for both reading and writing. The file must exist.
"w+"	Opens an empty file for both reading and writing. If the file exists, its contents are destroyed.
"a+"	Opens for reading and appending. Creates the file if it does not exist.

"x" will throw EINVAL which is does not satisfy POSIX, C11 and C++17 standard.
*/
	using utype = typename std::underlying_type<open_mode>::type;
	switch(static_cast<utype>(native_c_supported(m)))
	{
//Action if file already exists;	Action if file does not exist;	c-style mode;	Explanation
//Read from start;	Failure to open;	"r";	Open a file for reading
	case static_cast<utype>(open_mode::in)|static_cast<utype>(open_mode::text):
		return "\x72";
//Destroy contents;	Create new;	"w";	Create a file for writing
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::text):
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::trunc)|static_cast<utype>(open_mode::text):
		return "\x77";
//Append to file;	Create new;	"a";	Append to a file
	case static_cast<utype>(open_mode::app)|static_cast<utype>(open_mode::text):
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::app)|static_cast<utype>(open_mode::text):
		return "\x61";
//Read from start;	Error;	"r+";		Open a file for read/write
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::in)|static_cast<utype>(open_mode::text):
		return "\x72\x2b";
//Destroy contents;	Create new;	"w+";	Create a file for read/write
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::in)|static_cast<utype>(open_mode::trunc)|static_cast<utype>(open_mode::text):
		return "\x77\x2b";
//Write to end;	Create new;	"a+";	Open a file for read/write
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::in)|static_cast<utype>(open_mode::app)|static_cast<utype>(open_mode::text):
	case static_cast<utype>(open_mode::in)|static_cast<utype>(open_mode::app)|static_cast<utype>(open_mode::text):
		return "\x77\x2b";

//binary support

//Action if file already exists;	Action if file does not exist;	c-style mode;	Explanation
//Read from start;	Failure to open;	"rb";	Open a file for reading
	case static_cast<utype>(open_mode::in):
		return "\x72\x62";
//Destroy contents;	Create new;	"wb";	Create a file for writing
	case static_cast<utype>(open_mode::out):
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::trunc):
		return "\x77\x62";
//Append to file;	Create new;	"ab";	Append to a file
	case static_cast<utype>(open_mode::app):
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::app):
		return "\x61\x62";
//Read from start;	Error;	"r+b";		Open a file for read/write
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::in):
		return "\x72\x2b\x62";
//Destroy contents;	Create new;	"w+b";	Create a file for read/write
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::in)|static_cast<utype>(open_mode::trunc):
		return "\x77\x2b\x62";
//Write to end;	Create new;	"a+b";	Open a file for read/write
	case static_cast<utype>(open_mode::out)|static_cast<utype>(open_mode::in)|static_cast<utype>(open_mode::app):
	case static_cast<utype>(open_mode::in)|static_cast<utype>(open_mode::app):
		return "\x61\x2b\x62";
	case 0:
		if((m&open_mode::directory)!=open_mode::none)
			return "\x72";
		[[fallthrough]];
	default:
		return "";
	}
#else
	return to_c_mode(m);
#endif
}

namespace details
{

#if defined(__MSDOS__)
extern int fileno(FILE*) noexcept asm("_fileno");
extern FILE* fdopen(int,char const*) noexcept asm("_fdopen");
#elif defined(__CYGWIN__)
[[gnu::dllimport]] extern int fileno(FILE*) noexcept 
#if SIZE_MAX<=UINT32_MAX &&(defined(__x86__) || defined(_M_IX86) || defined(__i386__))
#if defined(__GNUC__)
asm("fileno")
#else
asm("_fileno")
#endif
#else
asm("fileno")
#endif
;
[[gnu::dllimport]] extern FILE* fdopen(int,char const*) noexcept
#if SIZE_MAX<=UINT32_MAX &&(defined(__x86__) || defined(_M_IX86) || defined(__i386__))
#if defined(__GNUC__)
asm("fdopen")
#else
asm("_fdopen")
#endif
#else
asm("fdopen")
#endif
;

#endif


#if defined(__CYGWIN__)

[[gnu::dllimport]] extern void my_cygwin_pthread_mutex_lock(void*) noexcept
#if SIZE_MAX<=UINT32_MAX &&(defined(__x86__) || defined(_M_IX86) || defined(__i386__))
#if defined(__GNUC__)
asm("pthread_mutex_lock")
#else
asm("_pthread_mutex_lock")
#endif
#else
asm("pthread_mutex_lock")
#endif
;

[[gnu::dllimport]] extern void my_cygwin_pthread_mutex_unlock(void*) noexcept
#if SIZE_MAX<=UINT32_MAX &&(defined(__x86__) || defined(_M_IX86) || defined(__i386__))
#if defined(__GNUC__)
asm("pthread_mutex_unlock")
#else
asm("_pthread_mutex_unlock")
#endif
#else
asm("pthread_mutex_unlock")
#endif
;

inline void my_cygwin_flockfile(FILE* fp) noexcept
{
	if(!((fp->_flags)&__SSTR))
		my_cygwin_pthread_mutex_lock(fp->_lock);
}

inline void my_cygwin_funlockfile(FILE* fp) noexcept
{
	if(!((fp->_flags)&__SSTR))
		my_cygwin_pthread_mutex_unlock(fp->_lock);
}

#endif

}


enum class c_family:char8_t
{
standard,
unlocked
};

namespace details
{

template<c_family family>
inline int my_fileno_impl(FILE* fp) noexcept
{
	if(fp==nullptr)
		return -1;
	if constexpr(family==c_family::standard)
	{
	return 
#if defined(_WIN32) && !defined(__CYGWIN__)
		noexcept_call(_fileno,fp)
#elif defined(__NEWLIB__)
		fp->_file
#else
		noexcept_call(fileno,fp)
#endif
	;

	}
	else
	{
	return 
#if defined(_WIN32) && !defined(__CYGWIN__)
		noexcept_call(_fileno,fp)
#elif defined(__NEWLIB__) || defined(__DARWIN_C_LEVEL)
		fp->_file
#elif defined(__MISC_VISIBLE) || defined(__USE_MISC)
		noexcept_call(fileno_unlocked,fp)
#else
		noexcept_call(fileno,fp)
#endif
	;
	}
}

inline int fp_to_fd(FILE* fp) noexcept
{
	return my_fileno_impl<c_family::standard>(fp);
}

inline int fp_unlocked_to_fd(FILE* fp) noexcept
{
	return my_fileno_impl<c_family::unlocked>(fp);
}

#if defined(_WIN32) && !defined(__CYGWIN__)
template<c_family family>
inline void* my_fp_to_win32_handle_impl(FILE* fp) noexcept
{
	return my_get_osfile_handle(my_fileno_impl<family>(fp));
}
#endif

template<c_family family>
inline int my_fclose_impl(FILE* fp) noexcept
{
	if constexpr(family==c_family::standard)
	{
#ifdef __has_builtin
#if __has_builtin(__builtin_fclose)
		return __builtin_fclose(fp);
#else
		return fclose(fp);
#endif
#else
		return fclose(fp);
#endif
	}
	else
	{
#if defined(_MSC_VER) || defined(_UCRT)
		return noexcept_call(_fclose_nolock,fp);
#else
#ifdef __has_builtin
#if __has_builtin(__builtin_fclose)
		return __builtin_fclose(fp);
#else
		return fclose(fp);
#endif
#else
		return fclose(fp);
#endif
#endif
	}
}

inline FILE* my_fdopen(int fd,char const* mode) noexcept
{
	auto fp{
#if defined(_WIN32) && !defined(__CYGWIN__)
		noexcept_call(_fdopen,fd,mode)
#elif defined(__MSDOS__) || defined(__CYGWIN__)
		fdopen(fd,mode)
#else
		noexcept_call(fdopen,fd,mode)
#endif
	};
	return fp;
}

inline FILE* my_c_file_open_impl(int fd,open_mode mode) noexcept
{
	char const* cmode{to_native_c_mode(mode)};
#if defined(__NEWLIB__) && !defined(__CYGWIN__)
	struct _reent ent{};
	auto fp{noexcept_call(_fdopen_r,__builtin_addressof(ent),fd,cmode)};
	if(fp==nullptr)
		throw_posix_error(ent._errno);
#else
	auto fp{my_fdopen(fd,cmode)};
	if(fp==nullptr)
		throw_posix_error();
#endif
	return fp;
}
#if defined(__NEWLIB__)
inline void my_c_io_newlib_flush_impl(FILE* fp)
{
	struct _reent ent{};
	if(noexcept_call(_fflush_r,__builtin_addressof(ent),fp))
		throw_posix_error(ent._errno);
}
#endif

template<c_family family>
inline void my_c_io_flush_impl(FILE* fp)
{
#if defined(__NEWLIB__) && !defined(__CYGWIN__)
	my_c_io_newlib_flush_impl(fp);
#else
	if constexpr(family==c_family::standard)
	{
#if defined(__has_builtin)
#if __has_builtin(__builtin_fflush)
		if(__builtin_flush(fp))
#else
		if(fflush(fp))
#endif
#else
		if(fflush(fp))
#endif
			throw_posix_error();
	}
	else
	{
#if defined(_MSC_VER) || defined(_UCRT)
		if(noexcept_call(_fflush_nolock,fp))
			throw_posix_error();
#elif defined(__MISC_VISIBLE) && !defined(__NEWLIB__)
		if(noexcept_call(fflush_unlocked,fp))
			throw_posix_error();
#else
		return my_c_io_flush_impl<c_family::standard>(fp);
#endif			
	}
#endif
}

inline void c_flush_unlocked_impl(FILE* fp)
{
	my_c_io_flush_impl<c_family::unlocked>(fp);
}

template<c_family family>
inline std::uintmax_t my_c_io_seek_impl(FILE* fp,std::intmax_t offset,seekdir s)
{

/*
We avoid standard C functions since they cannot deal with large file on 32 bits platforms

Reference:

https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fseek-nolock-fseeki64-nolock?view=vs-2019

https://www.gnu.org/software/libc/manual/html_node/File-Positioning.html

*/
	if constexpr(family==c_family::unlocked)
	{
#if defined(_WIN32) && !defined(__CYGWIN__)
#if defined(_MSC_VER) || defined(_UCRT)  || __MSVCRT_VERSION__ >= 0x800
		if(noexcept_call(_fseeki64_nolock,fp,offset,static_cast<int>(s)))
			throw_posix_error();
		auto val{noexcept_call(_ftelli64_nolock,fp)};
		if(val<0)
			throw_posix_error();
		return val;
#else
		if(noexcept_call(fseeko64,fp,offset,static_cast<int>(s)))
			throw_posix_error();
		auto val{noexcept_call(ftello64,fp)};
		if(val<0)
			throw_posix_error();
		return val;
#endif
#else
		return my_c_io_seek_impl<c_family::standard>(fp,offset,s);
#endif
	}
	else
	{

#if defined(__NEWLIB__) && !defined(__CYGWIN__)
		struct _reent ent{};
		if(noexcept_call(_fseeko_r,__builtin_addressof(ent),fp,static_cast<_off_t>(offset),static_cast<int>(s)))
			throw_posix_error(ent._errno);
		ent={};
		auto val{noexcept_call(_ftell_r,__builtin_addressof(ent),fp)};
		if(val<0)
			throw_posix_error(ent._errno);
		return val;
#else
		if(
#if defined(_WIN32) && !defined(__CYGWIN__)
		_fseeki64(fp,offset,static_cast<int>(s))
#elif defined(__USE_LARGEFILE64)
		noexcept_call(fseeko64,fp,offset,static_cast<int>(s))
#elif defined(__has_builtin)
#if __has_builtin(__builtin_fseek)
		__builtin_fseek(fp,offset,static_cast<int>(s))
#else
		fseek(fp,offset,static_cast<int>(s))
#endif
#else
		fseek(fp,offset,static_cast<int>(s))
#endif
		)
			throw_posix_error();
		auto val{
#if defined(_WIN32) && !defined(__CYGWIN__)
		noexcept_call(_ftelli64,fp)
#elif defined(__USE_LARGEFILE64)
		noexcept_call(ftello64,fp)
#elif defined(__has_builtin)
#if __has_builtin(__builtin_ftell)
		__builtin_ftell(fp)
#else
		ftell(fp)
#endif
#else
		ftell(fp)
#endif
		};
		if(val<0)
			throw_posix_error();
		return val;
#endif
	}
}

}

template<c_family family,std::integral ch_type>
requires (family==c_family::standard||family==c_family::unlocked)
class basic_c_family_io_observer
{
public:
	using char_type = ch_type;
	using native_handle_type = FILE*;
	native_handle_type fp{};
	constexpr native_handle_type& native_handle() noexcept
	{
		return fp;
	}
	constexpr native_handle_type const& native_handle() const noexcept
	{
		return fp;
	}
	explicit constexpr operator bool() const noexcept
	{
		return fp;
	}
	constexpr native_handle_type release() noexcept
	{
		auto temp{fp};
		fp=nullptr;
		return temp;
	}
	explicit operator basic_posix_io_observer<char_type>() const noexcept
	{
		return basic_posix_io_observer<char_type>{details::my_fileno_impl<family>(fp)};
	}
#if defined(_WIN32) && !defined(__CYGWIN__)
	template<win32_family fam>
	explicit operator basic_win32_family_io_observer<fam,char_type>() const noexcept
	{
		return static_cast<basic_win32_family_io_observer<fam,char_type>>(details::my_fp_to_win32_handle_impl<family>(fp));
	}
	template<nt_family fam>
	explicit operator basic_nt_family_io_observer<fam,char_type>() const noexcept
	{
		return static_cast<basic_nt_family_io_observer<fam,char_type>>(details::my_fp_to_win32_handle_impl<family>(fp));
	}
#endif
	inline void lock() const noexcept requires(family==c_family::standard)
	{
#if (defined(_MSC_VER)||defined(_UCRT)) && !defined(__CYGWIN__)
	noexcept_call(_lock_file,fp);
#elif defined(_WIN32) && !defined(__CYGWIN__)
	win32::my_msvcrt_lock_file(fp);
#elif !defined(__SINGLE_THREAD__)
#if defined(__NEWLIB__)
#if defined(__CYGWIN__)
	details::my_cygwin_flockfile(fp);
#elif !defined(__SINGLE_THREAD__)
//	_flockfile(fp);	//TO FIX undefined reference to `__cygwin_lock_lock' why?
#endif
#elif defined(__MSDOS__) || (defined(__wasi__) &&!defined(__wasilibc_unmodified_upstream) && !defined(_REENTRANT)) || defined(__mlibc__)
#else
	noexcept_call(flockfile,fp);
#endif
#endif
	}
	inline void unlock() const noexcept requires(family==c_family::standard)
	{
#if (defined(_MSC_VER)||defined(_UCRT)) && !defined(__CYGWIN__)
	noexcept_call(_unlock_file,fp);
#elif defined(_WIN32) && !defined(__CYGWIN__)
	win32::my_msvcrt_unlock_file(fp);
#elif !defined(__SINGLE_THREAD__)
#if defined(__NEWLIB__)
#if defined(__CYGWIN__)
	details::my_cygwin_funlockfile(fp);
#elif !defined(__SINGLE_THREAD__)
//	_funlockfile(fp); //TO FIX
#endif
#elif defined(__MSDOS__) || (defined(__wasi__) &&!defined(__wasilibc_unmodified_upstream) && !defined(_REENTRANT)) || defined(__mlibc__)
#else
	noexcept_call(funlockfile,fp);
#endif
#endif
	}
	inline constexpr basic_c_family_io_observer<c_family::unlocked,ch_type> unlocked_handle() const noexcept requires(family==c_family::standard)
	{
		return {fp};
	}
};

template<c_family family,std::integral ch_type>
inline constexpr posix_at_entry at(basic_c_family_io_observer<family,ch_type> other) noexcept
{
	return posix_at_entry{details::my_fileno_impl<family>(other.fp)};
}

template<c_family family,std::integral ch_type>
inline constexpr basic_c_family_io_observer<family,ch_type> io_value_handle(basic_c_family_io_observer<family,ch_type> other) noexcept
{
	return other;
}

template<c_family family,std::integral ch_type>
requires requires(basic_posix_io_observer<ch_type> piob)
{
	status(piob);
}
inline constexpr posix_file_status status(basic_c_family_io_observer<family,ch_type> ciob)
{
	return status(static_cast<basic_posix_io_observer<ch_type>>(ciob));
}

template<c_family family,std::integral ch_type>
inline void flush(basic_c_family_io_observer<family,ch_type> cfhd)
{
	details::my_c_io_flush_impl<family>(cfhd.fp);
}

template<c_family family,std::integral ch_type,typename... Args>
requires io_controllable<basic_posix_io_observer<ch_type>,Args...>
inline decltype(auto) io_control(basic_c_family_io_observer<family,ch_type> h,Args&& ...args)
{
	return io_control(static_cast<basic_posix_io_observer<ch_type>>(h),std::forward<Args>(args)...);
}

template<c_family family,std::integral ch_type>
inline std::uintmax_t seek(basic_c_family_io_observer<family,ch_type> cfhd,std::intmax_t offset=0,seekdir s=seekdir::cur)
{
	return details::my_c_io_seek_impl<family>(cfhd.fp,offset,s);
}

#if __cpp_lib_three_way_comparison >= 201907L

template<c_family family,std::integral ch_type>
inline constexpr bool operator==(basic_c_family_io_observer<family,ch_type> a,basic_c_family_io_observer<family,ch_type> b) noexcept
{
	return a.fp==b.fp;
}

template<c_family family,std::integral ch_type>
inline constexpr auto operator<=>(basic_c_family_io_observer<family,ch_type> a,basic_c_family_io_observer<family,ch_type> b) noexcept
{
	return a.fp<=>b.fp;
}
#endif


namespace details
{

template<c_family family>
inline bool my_c_is_character_device_impl(FILE* fp) noexcept
{
	return posix_is_character_device(my_fileno_impl<family>(fp));
}


template<c_family family>
inline void my_c_clear_screen_impl(FILE* fp)
{
	if constexpr(family==c_family::standard)
	{
		basic_c_family_io_observer<c_family::standard,char> ciob{fp};
		lock_guard guard{ciob};
		my_c_clear_screen_impl<c_family::unlocked>(fp);
	}
	else
	{
#ifdef _WIN32
		void* handle{my_fp_to_win32_handle_impl<c_family::unlocked>(fp)};
		if(!::fast_io::win32::details::win32_is_character_device(handle))
			return;
		my_c_io_flush_impl<c_family::unlocked>(fp);
		::fast_io::win32::details::win32_clear_screen_main(handle);
#else
		int fd{my_fileno_impl<c_family::unlocked>(fp)};
		if(!posix_is_character_device(fd))
			return;
		my_c_io_flush_impl<c_family::unlocked>(fp);
		posix_clear_screen_main(fd);
#endif
	}
}

}

template<c_family family,std::integral ch_type>
inline bool is_character_device(basic_c_family_io_observer<family,ch_type> ciob) noexcept
{
	return details::my_c_is_character_device_impl<family>(ciob.fp);
}

template<c_family family,std::integral ch_type>
inline void clear_screen(basic_c_family_io_observer<family,ch_type> ciob)
{
	details::my_c_clear_screen_impl<family>(ciob.fp);
}

template<c_family family,std::integral ch_type>
requires requires(basic_c_family_io_observer<family,ch_type> h)
{
	redirect_handle(static_cast<basic_posix_io_observer<ch_type>>(h));
}
inline decltype(auto) redirect_handle(basic_c_family_io_observer<family,ch_type> h)
{
	return redirect_handle(static_cast<basic_posix_io_observer<ch_type>>(h));
}

template<c_family family,std::integral ch_type>
requires zero_copy_input_stream<basic_posix_io_observer<ch_type>>
inline decltype(auto) zero_copy_in_handle(basic_c_family_io_observer<family,ch_type> h)
{
	return zero_copy_in_handle(static_cast<basic_posix_io_observer<ch_type>>(h));
}

template<c_family family,std::integral ch_type>
requires zero_copy_output_stream<basic_posix_io_observer<ch_type>>
inline decltype(auto) zero_copy_out_handle(basic_c_family_io_observer<family,ch_type> h)
{
	return zero_copy_out_handle(static_cast<basic_posix_io_observer<ch_type>>(h));
}

template<c_family family,std::integral ch_type>
class basic_c_family_io_handle:public basic_c_family_io_observer<family,ch_type>
{
public:
	using char_type = ch_type;
	using native_handle_type = FILE*;
	constexpr basic_c_family_io_handle() noexcept=default;
	template<typename native_hd>
	requires std::same_as<native_handle_type,std::remove_cvref_t<native_hd>>
	explicit constexpr basic_c_family_io_handle(native_hd fp) noexcept:basic_c_family_io_observer<family,ch_type>{fp}{}

	basic_c_family_io_handle(basic_c_family_io_handle const&)=delete;
	basic_c_family_io_handle& operator=(basic_c_family_io_handle const&)=delete;
	constexpr basic_c_family_io_handle(basic_c_family_io_handle&& other) noexcept:basic_c_family_io_observer<family,ch_type>{other.fp}
	{
		other.fp=nullptr;
	}
	basic_c_family_io_handle& operator=(basic_c_family_io_handle&& other) noexcept
	{
		if(__builtin_addressof(other)!=this)
			return *this;
		if(this->fp)[[likely]]
		{
#if defined(__NEWLIB__) && !defined(__CYGWIN__)
			struct _reent ent{};
			noexcept_call(_fclose_r,__builtin_addressof(ent),this->fp);
#else
			details::my_fclose_impl<family>(this->fp);
#endif
		}
		this->fp=other.fp;
		other.fp=nullptr;
		return *this;
	}
	void close()
	{
		if(this->fp==nullptr)[[unlikely]]
			return;
#if defined(__NEWLIB__) && !defined(__CYGWIN__)
		struct _reent ent{};
		int ret{noexcept_call(_fclose_r,__builtin_addressof(ent),this->fp)};
		this->fp=nullptr;
		if(ret==EOF)
			throw_posix_error(ent._errno);
#else
		int ret{details::my_fclose_impl<family>(this->fp)};
		this->fp=nullptr;
		if(ret==EOF)
			throw_posix_error(errno);
#endif
	}
};

template<c_family family>
struct
#if __has_cpp_attribute(gnu::trivial_abi)
[[gnu::trivial_abi]]
#endif
c_family_file_factory
{
	using native_handle_type = FILE*;
	FILE* fp{};
	explicit constexpr c_family_file_factory(FILE* fpp) noexcept:fp(fpp){};
	c_family_file_factory(c_family_file_factory const&)=delete;
	c_family_file_factory& operator=(c_family_file_factory const&)=delete;
	~c_family_file_factory()
	{
		if(fp)[[likely]]
		{
#if defined(__NEWLIB__) && !defined(__CYGWIN__)
			struct _reent ent{};
			noexcept_call(_fclose_r,__builtin_addressof(ent),this->fp);
#else
			details::my_fclose_impl<family>(this->fp);
#endif
		}
	}
};

template<c_family family,std::integral ch_type>
class basic_c_family_file:public basic_c_family_io_handle<family,ch_type>
{
public:
	using char_type = ch_type;
	using native_handle_type = FILE*;
	constexpr basic_c_family_file() noexcept=default;
	template<typename native_hd>
	requires std::same_as<native_handle_type,std::remove_cvref_t<native_hd>>
	explicit constexpr basic_c_family_file(native_hd fp) noexcept:basic_c_family_io_handle<family,ch_type>{fp}{}
	template<c_family family2>
	explicit constexpr basic_c_family_file(c_family_file_factory<family2>&& other) noexcept:basic_c_family_io_handle<family,ch_type>{other.fp}
	{
		other.fp=nullptr;
	}
	basic_c_family_file(basic_c_family_file const&)=delete;
	basic_c_family_file& operator=(basic_c_family_file const&)=delete;
	constexpr basic_c_family_file(basic_c_family_file&&) noexcept=default;
	basic_c_family_file& operator=(basic_c_family_file&&) noexcept=default;
	~basic_c_family_file()
	{
		if(this->fp)[[likely]]
		{
#if defined(__NEWLIB__) && !defined(__CYGWIN__)
			struct _reent ent{};
			noexcept_call(_fclose_r,__builtin_addressof(ent),this->fp);
#else
			details::my_fclose_impl<family>(this->fp);
#endif
		}
	}
	basic_c_family_file(basic_posix_io_handle<char_type>&& phd,open_mode om):basic_c_family_io_handle<family,ch_type>{details::my_c_file_open_impl(phd.fd,om)}
	{
		phd.fd=-1;
	}
#if defined(_WIN32) && !defined(__CYGWIN__)
//windows specific. open posix file from win32 io handle
	template<win32_family wfamily>
	basic_c_family_file(basic_win32_family_io_handle<wfamily,char_type>&& win32_handle,open_mode om):
		basic_c_family_file(basic_posix_file<char_type>(std::move(win32_handle),om),to_native_c_mode(om))
	{
	}
	template<nt_family nfamily>
	basic_c_family_file(basic_nt_family_io_handle<nfamily,char_type>&& nt_handle,open_mode om):
		basic_c_family_file(basic_posix_file<char_type>(std::move(nt_handle),om),to_native_c_mode(om))
	{
	}
#endif
	basic_c_family_file(native_fs_dirent ent,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(ent,om,pm),om)
	{}
	basic_c_family_file(cstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(file,om,pm),om)
	{}
	basic_c_family_file(native_at_entry nate,cstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(nate,file,om,pm),om)
	{}
	basic_c_family_file(wcstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(file,om,pm),om)
	{}
	basic_c_family_file(native_at_entry nate,wcstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(nate,file,om,pm),om)
	{}
	basic_c_family_file(u8cstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(file,om,pm),om)
	{}
	basic_c_family_file(native_at_entry nate,u8cstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(nate,file,om,pm),om)
	{}
	basic_c_family_file(u16cstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(file,om,pm),om)
	{}
	basic_c_family_file(native_at_entry nate,u16cstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(nate,file,om,pm),om)
	{}
	basic_c_family_file(u32cstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(file,om,pm),om)
	{}
	basic_c_family_file(native_at_entry nate,u32cstring_view file,open_mode om,perms pm=static_cast<perms>(436)):
		basic_c_family_file(basic_posix_file<char_type>(nate,file,om,pm),om)
	{}
};

template<std::integral char_type>
using basic_c_io_observer_unlocked = basic_c_family_io_observer<c_family::unlocked,char_type>;
template<std::integral char_type>
using basic_c_io_observer = basic_c_family_io_observer<c_family::standard,char_type>;

template<std::integral char_type>
using basic_c_io_handle_unlocked = basic_c_family_io_handle<c_family::unlocked,char_type>;
template<std::integral char_type>
using basic_c_io_handle = basic_c_family_io_handle<c_family::standard,char_type>;

template<std::integral char_type>
using basic_c_file_unlocked = basic_c_family_file<c_family::unlocked,char_type>;
template<std::integral char_type>
using basic_c_file = basic_c_family_file<c_family::standard,char_type>;

using c_io_observer_unlocked=basic_c_io_observer_unlocked<char>;
using c_io_observer=basic_c_io_observer<char>;
using c_io_handle_unlocked = basic_c_io_handle_unlocked<char>;
using c_io_handle = basic_c_io_handle<char>;
using c_file = basic_c_file<char>;
using c_file_unlocked = basic_c_file_unlocked<char>;
using wc_io_observer_unlocked=basic_c_io_observer_unlocked<wchar_t>;
using wc_io_observer=basic_c_io_observer<wchar_t>;
using wc_io_handle_unlocked = basic_c_io_handle_unlocked<wchar_t>;
using wc_io_handle = basic_c_io_handle<wchar_t>;
using wc_file = basic_c_file<wchar_t>;
using wc_file_unlocked = basic_c_file_unlocked<wchar_t>;
using u8c_io_observer_unlocked=basic_c_io_observer_unlocked<char8_t>;
using u8c_io_observer=basic_c_io_observer<char8_t>;
using u8c_io_handle_unlocked = basic_c_io_handle_unlocked<char8_t>;
using u8c_io_handle = basic_c_io_handle<char8_t>;
using u8c_file = basic_c_file<char8_t>;
using u8c_file_unlocked = basic_c_file_unlocked<char8_t>;
using u16c_io_observer_unlocked=basic_c_io_observer_unlocked<char16_t>;
using u16c_io_observer=basic_c_io_observer<char16_t>;
using u16c_io_handle_unlocked = basic_c_io_handle_unlocked<char16_t>;
using u16c_io_handle = basic_c_io_handle<char16_t>;
using u16c_file = basic_c_file<char16_t>;
using u16c_file_unlocked = basic_c_file_unlocked<char16_t>;
using u32c_io_observer_unlocked = basic_c_io_observer_unlocked<char32_t>;
using u32c_io_observer=basic_c_io_observer<char32_t>;
using u32c_io_handle_unlocked = basic_c_io_handle_unlocked<char32_t>;
using u32c_io_handle = basic_c_io_handle<char32_t>;
using u32c_file = basic_c_file<char32_t>;
using u32c_file_unlocked = basic_c_file_unlocked<char32_t>;

using c_file_factory = c_family_file_factory<c_family::standard>;
using c_file_factory_unlocked = c_family_file_factory<c_family::unlocked>;

}

#if defined(_WIN32) && !defined(__CYGWIN__)
#include"wincrt.h"
#else
#if defined(__UCLIBC__)
#if defined(__STDIO_BUFFERS)
#include"uclibc.h"
#elif defined(FAST_IO_LIBC_CUSTOM_BUFFER_PTRS)
#include"custom.h"
#endif
#elif defined(__mlibc__)
#include"mlibc.h"
#elif defined(__GLIBC__)
#include"glibc.h"
#elif defined(__wasi__)
#include"musl.h"
#elif defined(__NEED___isoc_va_list) || defined(__musl__)
#include"musl.h"
#elif defined(__BSD_VISIBLE) ||defined(__DARWIN_C_LEVEL) \
	|| (defined(__NEWLIB__) &&!defined(__CUSTOM_FILE_IO__)) \
	|| defined(__BIONIC__) || defined(__MSDOS__)
#include"unix.h"
#elif defined(FAST_IO_LIBC_CUSTOM_BUFFER_PTRS)
#include"custom.h"
#endif

#if !defined(__MSDOS__)
#include"general.h"
#endif
#include"done.h"
#endif


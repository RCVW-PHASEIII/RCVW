
/**
 * WARNING: DO NOT EDIT THIS FILE
 *
 * This source file was generated from the CPlusPlusGenerator.xsl stylesheet
 *
 * Your changes will be overwritten when this tool executes again.  In order
 * to edit the contents, you must go into the source XML files
 *
 * @author Greg Baumgardner
 */
 

#ifndef INCLUDE_TMXLOG_H_
#define INCLUDE_TMXLOG_H_

#ifndef CONSTEXPR
#if __cplusplus > 199711L
#define CONSTEXPR constexpr
#else
#ifdef __GNUC__
#define CONSTEXPR __attribute__ ((unused))
#else
#define CONSTEXPR
#endif /* __GNUC__ */
#endif /* __cplusplus > 199711L */
#endif

#if __cplusplus >= 201103L
#include <tmx/utils/Enum.hpp>
#endif /* __cplusplus >= 201103L */
		
	
namespace tmx {
	
		
namespace utils {
	
			
enum class LogLevel 
{
		logERROR = 0,
		logWARNING = 1,
		logINFO = 2,
		logDEBUG = 3,
		logDEBUG1 = 4,
		logDEBUG2 = 5,
		logDEBUG3 = 6,
		logDEBUG4 = 7
};
		
static CONSTEXPR const char *LOGLEVEL_LOGERROR_STRING = "ERROR";
static CONSTEXPR const char *LOGLEVEL_LOGWARNING_STRING = "WARNING";
static CONSTEXPR const char *LOGLEVEL_LOGINFO_STRING = "INFO";
static CONSTEXPR const char *LOGLEVEL_LOGDEBUG_STRING = "DEBUG";
static CONSTEXPR const char *LOGLEVEL_LOGDEBUG1_STRING = "DEBUG1";
static CONSTEXPR const char *LOGLEVEL_LOGDEBUG2_STRING = "DEBUG2";
static CONSTEXPR const char *LOGLEVEL_LOGDEBUG3_STRING = "DEBUG3";
static CONSTEXPR const char *LOGLEVEL_LOGDEBUG4_STRING = "DEBUG4";
		
static CONSTEXPR const char *LOGLEVEL_ALL_STRINGS[] = 
{
		LOGLEVEL_LOGERROR_STRING,
		LOGLEVEL_LOGWARNING_STRING,
		LOGLEVEL_LOGINFO_STRING,
		LOGLEVEL_LOGDEBUG_STRING,
		LOGLEVEL_LOGDEBUG1_STRING,
		LOGLEVEL_LOGDEBUG2_STRING,
		LOGLEVEL_LOGDEBUG3_STRING,
		LOGLEVEL_LOGDEBUG4_STRING
};
		
		
} /* End namespace utils */
		
	
} /* End namespace tmx */
		

#if __cplusplus >= 201103L
namespace tmx {
		
template <tmx::utils::LogLevel V> struct EnumName<tmx::utils::LogLevel, V> {
	static constexpr const char *name = tmx::utils::LOGLEVEL_ALL_STRINGS[static_cast<size_t>(V)];
};
template <> struct EnumSequenceBuilder<tmx::utils::LogLevel> {
	typedef EnumSequence<tmx::utils::LogLevel,
		tmx::utils::LogLevel::logERROR,
		tmx::utils::LogLevel::logWARNING,
		tmx::utils::LogLevel::logINFO,
		tmx::utils::LogLevel::logDEBUG,
		tmx::utils::LogLevel::logDEBUG1,
		tmx::utils::LogLevel::logDEBUG2,
		tmx::utils::LogLevel::logDEBUG3,
		tmx::utils::LogLevel::logDEBUG4> type;
};
	
} /* End namespace tmx */
#endif /* __cplusplus >= 201103L */ 
#endif /* INCLUDE_TMXLOG_H_ */
	
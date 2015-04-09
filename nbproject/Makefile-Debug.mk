#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-MacOSX
CND_DLIB_EXT=dylib
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/AssignedTrip.o \
	${OBJECTDIR}/CSVRow.o \
	${OBJECTDIR}/DataContainer.o \
	${OBJECTDIR}/Driver.o \
	${OBJECTDIR}/MitmModel.o \
	${OBJECTDIR}/OpenTrip.o \
	${OBJECTDIR}/Output.o \
	${OBJECTDIR}/Request.o \
	${OBJECTDIR}/Rider.o \
	${OBJECTDIR}/Solution.o \
	${OBJECTDIR}/TripData.o \
	${OBJECTDIR}/UFBW_fixed.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../../Documents/3rdparty/or-tools.MacOsX64/lib -L../../Documents/3rdparty/COIN/Cbc-2.3.2-mac-osx-x86-icc10.1-parallel/lib -lortools

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uberpoolmatchopt

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uberpoolmatchopt: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uberpoolmatchopt ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/AssignedTrip.o: AssignedTrip.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AssignedTrip.o AssignedTrip.cpp

${OBJECTDIR}/CSVRow.o: CSVRow.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CSVRow.o CSVRow.cpp

${OBJECTDIR}/DataContainer.o: DataContainer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DataContainer.o DataContainer.cpp

${OBJECTDIR}/Driver.o: Driver.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Driver.o Driver.cpp

${OBJECTDIR}/MitmModel.o: MitmModel.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MitmModel.o MitmModel.cpp

${OBJECTDIR}/OpenTrip.o: OpenTrip.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OpenTrip.o OpenTrip.cpp

${OBJECTDIR}/Output.o: Output.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Output.o Output.cpp

${OBJECTDIR}/Request.o: Request.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Request.o Request.cpp

${OBJECTDIR}/Rider.o: Rider.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Rider.o Rider.cpp

${OBJECTDIR}/Solution.o: Solution.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Solution.o Solution.cpp

${OBJECTDIR}/TripData.o: TripData.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TripData.o TripData.cpp

${OBJECTDIR}/UFBW_fixed.o: UFBW_fixed.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UFBW_fixed.o UFBW_fixed.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -I../../Documents/3rdparty/COIN/2.9.4/Cbc/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uberpoolmatchopt

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

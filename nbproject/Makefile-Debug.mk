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
	${OBJECTDIR}/AssignedRoute.o \
	${OBJECTDIR}/AssignedTrip.o \
	${OBJECTDIR}/CSVRow.o \
	${OBJECTDIR}/DataContainer.o \
	${OBJECTDIR}/Driver.o \
	${OBJECTDIR}/FlexDepSolution.o \
	${OBJECTDIR}/FlexDepartureModel.o \
	${OBJECTDIR}/GenerateInstanceScenarios.o \
	${OBJECTDIR}/Geofence.o \
	${OBJECTDIR}/MitmModel.o \
	${OBJECTDIR}/ModelRunner.o \
	${OBJECTDIR}/MultPickupSoln.o \
	${OBJECTDIR}/MultiplePickupsModel.o \
	${OBJECTDIR}/OpenTrip.o \
	${OBJECTDIR}/Output.o \
	${OBJECTDIR}/ProblemInstance.o \
	${OBJECTDIR}/Request.o \
	${OBJECTDIR}/Rider.o \
	${OBJECTDIR}/Route.o \
	${OBJECTDIR}/RouteEvent.o \
	${OBJECTDIR}/Solution.o \
	${OBJECTDIR}/TripData.o \
	${OBJECTDIR}/UFBW_fixed.o \
	${OBJECTDIR}/UFBW_perfectInformation.o \
	${OBJECTDIR}/UserConfig.o \
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

${OBJECTDIR}/AssignedRoute.o: AssignedRoute.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AssignedRoute.o AssignedRoute.cpp

${OBJECTDIR}/AssignedTrip.o: AssignedTrip.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AssignedTrip.o AssignedTrip.cpp

${OBJECTDIR}/CSVRow.o: CSVRow.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CSVRow.o CSVRow.cpp

${OBJECTDIR}/DataContainer.o: DataContainer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DataContainer.o DataContainer.cpp

${OBJECTDIR}/Driver.o: Driver.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Driver.o Driver.cpp

${OBJECTDIR}/FlexDepSolution.o: FlexDepSolution.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FlexDepSolution.o FlexDepSolution.cpp

${OBJECTDIR}/FlexDepartureModel.o: FlexDepartureModel.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FlexDepartureModel.o FlexDepartureModel.cpp

${OBJECTDIR}/GenerateInstanceScenarios.o: GenerateInstanceScenarios.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GenerateInstanceScenarios.o GenerateInstanceScenarios.cpp

${OBJECTDIR}/Geofence.o: Geofence.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Geofence.o Geofence.cpp

${OBJECTDIR}/MitmModel.o: MitmModel.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MitmModel.o MitmModel.cpp

${OBJECTDIR}/ModelRunner.o: ModelRunner.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ModelRunner.o ModelRunner.cpp

${OBJECTDIR}/MultPickupSoln.o: MultPickupSoln.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MultPickupSoln.o MultPickupSoln.cpp

${OBJECTDIR}/MultiplePickupsModel.o: MultiplePickupsModel.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MultiplePickupsModel.o MultiplePickupsModel.cpp

${OBJECTDIR}/OpenTrip.o: OpenTrip.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OpenTrip.o OpenTrip.cpp

${OBJECTDIR}/Output.o: Output.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Output.o Output.cpp

${OBJECTDIR}/ProblemInstance.o: ProblemInstance.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ProblemInstance.o ProblemInstance.cpp

${OBJECTDIR}/Request.o: Request.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Request.o Request.cpp

${OBJECTDIR}/Rider.o: Rider.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Rider.o Rider.cpp

${OBJECTDIR}/Route.o: Route.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Route.o Route.cpp

${OBJECTDIR}/RouteEvent.o: RouteEvent.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RouteEvent.o RouteEvent.cpp

${OBJECTDIR}/Solution.o: Solution.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Solution.o Solution.cpp

${OBJECTDIR}/TripData.o: TripData.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TripData.o TripData.cpp

${OBJECTDIR}/UFBW_fixed.o: UFBW_fixed.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UFBW_fixed.o UFBW_fixed.cpp

${OBJECTDIR}/UFBW_perfectInformation.o: UFBW_perfectInformation.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UFBW_perfectInformation.o UFBW_perfectInformation.cpp

${OBJECTDIR}/UserConfig.o: UserConfig.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UserConfig.o UserConfig.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Documents/3rdparty/or-tools.MacOsX64/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

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

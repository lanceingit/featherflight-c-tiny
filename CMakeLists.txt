cmake_minimum_required (VERSION 2.8)

project (ff)

ADD_DEFINITIONS(-DLINUX)

set(CMAKE_SYSTEM_NAME "linux")
set(CMAKE_C_COMPILER   "/home/neal/work/flyzero/out/sun8iw8p1/linux/common/buildroot/external-toolchain/bin/arm-linux-gnueabi-gcc")

set(CMAKE_C_FLAGS "-fno-math-errno \
                   -Os -mcpu=cortex-a7 \
				   -mfloat-abi=softfp \
				   -mfpu=neon \
				   -lrt \
				   -lpthread \
				   -ldl \
				   -lm \
				   -Wall \
				   -fno-strict-aliasing \
				   -Wdouble-promotion \
				   -std=gnu99" 
				   )

include_directories (src src/config src/bsp src/driver src/pilot src/pilot/navigator src/pilot/estimator)
include_directories (src/modules src/link src/mathlib src/utils src/param)
include_directories (src/link/mavlink src/link/mavlink/common src/link/wwlink)

set(config_module_list 
	src/main.c
	
	# driver
	src/driver/timer.c
	src/driver/mpu6050_linux.c
	src/driver/spl06_linux.c

	# module
	src/modules/log.c
	src/modules/param.c
	src/modules/mtd.c
	src/modules/cli.c
	src/modules/scheduler.c
	
	#param
	src/param/param_api.c

	# link
	src/link/link_mavlink.c
	src/link/link_wwlink.c
	src/link/mavlink_log.c
	
	#pilot
	src/pilot/est.c
	src/pilot/mixer.c
	src/pilot/commander.c
	src/pilot/att_control.c
	src/pilot/sensor.c
	src/pilot/navigator.c
	
	#estimator
	src/pilot/estimator/att_est_q.c
	src/pilot/estimator/att_est_cf.c
	src/pilot/estimator/alt_est_3o.c
	src/pilot/estimator/alt_est_inav.c

	#navgator
	src/pilot/navigator/stabilize.c

	#mathlib
	src/mathlib/mathlib.c
	src/mathlib/matrix.c
	src/mathlib/vector.c
	src/mathlib/quaternion.c
	src/mathlib/dcm.c
#	src/mathlib/srcdkf.c

	# utils
	src/utils/fifo.c
	src/utils/perf.c
	src/utils/lpf.c
	src/utils/pid.c
	src/utils/rotation.c
	
)

add_executable(ff ${config_module_list})

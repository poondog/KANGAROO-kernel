ccflags-y := -Ofast -ffast-math -fgcse-lm -fgcse-sm -fsched-spec-load -fforce-addr -fsingle-precision-constant -mtune=cortex-a9 -marm -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-double

obj-y			+= drm/ vga/ stub/ ion/

ifeq ($(CONFIG_MSM_KGSL_ADRENO200),y)
    obj-y += msm_adreno200/
endif

ifeq ($(CONFIG_MSM_KGSL_ADRENO205),y)
    obj-y += msm_adreno205/
endif

ifeq ($(CONFIG_MSM_KGSL_ADRENO220),y)
    obj-y += msm_adreno220/
endif

ifeq ($(CONFIG_MSM_KGSL_ADRENO225),y)
    obj-y += msm/
endif

package=libzerocash
$(package)_download_path=https://github.com/Electric-Coin-Company/$(package)/archive/
$(package)_file_name=$(package)-$($(package)_git_commit).tar.gz
$(package)_download_file=$($(package)_git_commit).tar.gz
$(package)_sha256_hash=082c27aed53a743a3041b3dc2cdba5bdd23839626dead0425517df613f5fc449
$(package)_git_commit=846e838a564f44d09866645cc3265ff39bc91804

$(package)_dependencies=libsnark crypto++ openssl boost libgmp
$(package)_patches=1_raw_keygen_api.patch

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/1_raw_keygen_api.patch && \
  rm libzerocash/allocators.h libzerocash/serialize.h libzerocash/streams.h
endef

# FIXME: How do we know, at the point where the _build_cms are run, that the
# $(host_prefix)/include/libsnark folder is there? The lifecycle of that folder
# is as follows:
# 	1. First, the _stage_cmds of libsnark.mk create it in the staging directory.
#	2. At some point in time, the depends system moves it from the staging
#	   directory to the actual $(host_prefix)/include/libsnark directory.
#
# If (2) happens after the libzerocash_build_cmds get run, then what's in the
# $(host_prefix)/include/libsnark directory will be an *old* copy of the
# libsnark headers, and we might have to build twice in order for libzerocash to
# get the changes. If (2) happens before, then all is well, and it works.
#
#                       ** Which is it? **
#
$(package)_cppflags += -I$(BASEDIR)/../src -I. -I$(host_prefix)/include -I$(host_prefix)/include/libsnark -DCURVE_ALT_BN128 -DBOOST_SPIRIT_THREADSAFE -DHAVE_BUILD_INFO -D__STDC_FORMAT_MACROS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -std=c++11 -pipe -O2 -O0 -g -Wstack-protector -fstack-protector-all -fPIE -fvisibility=hidden
$(package)_cppflags += -I$(BASEDIR)/../src -I. -I$(host_prefix)/include -I$(host_prefix)/include/libsnark -DCURVE_ALT_BN128 -DBOOST_SPIRIT_THREADSAFE -DHAVE_BUILD_INFO -D__STDC_FORMAT_MACROS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -std=c++11 -pipe -O2 -O0 -g -Wstack-protector -fstack-protector-all -fPIE -fvisibility=hidden -fPIC

define $(package)_build_cmds
  $(MAKE) all DEPINST=$(host_prefix) CXXFLAGS="$($(package)_cppflags)" MINDEPS=1 USE_MT=1
endef

define $(package)_stage_cmds
  $(MAKE) install STATIC=1 DEPINST=$(host_prefix) PREFIX=$($(package)_staging_dir)$(host_prefix) MINDEPS=1 USE_MT=1
endef

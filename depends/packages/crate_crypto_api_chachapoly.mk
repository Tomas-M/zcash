package=crate_crypto_api_chachapoly
$(package)_crate_name=crypto_api_chachapoly
$(package)_version=0.4.2
$(package)_download_path=https://static.crates.io/crates/$($(package)_crate_name)
$(package)_file_name=$($(package)_crate_name)-$($(package)_version).crate
$(package)_sha256_hash=19fc1fabcff68e7d49868d88eb65e6c314d85ea8307356c61d2a584160ba7775
$(package)_crate_versioned_name=$($(package)_crate_name)

define $(package)_preprocess_cmds
  $(call generate_crate_checksum,$(package))
endef

define $(package)_stage_cmds
  $(call vendor_crate_source,$(package))
endef

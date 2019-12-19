package=crate_semver_parser
$(package)_crate_name=semver-parser
$(package)_version=0.9.0
$(package)_download_path=https://static.crates.io/crates/$($(package)_crate_name)
$(package)_file_name=$($(package)_crate_name)-$($(package)_version).crate
$(package)_sha256_hash=b46e1121e8180c12ff69a742aabc4f310542b6ccb69f1691689ac17fdf8618aa
$(package)_crate_versioned_name=$($(package)_crate_name)

define $(package)_preprocess_cmds
  $(call generate_crate_checksum,$(package))
endef

define $(package)_stage_cmds
  $(call vendor_crate_source,$(package))
endef

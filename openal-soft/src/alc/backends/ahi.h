#ifndef BACKENDS_AHI_H
#define BACKENDS_AHI_H

#include "base.h"

struct AHIBackendFactory final : public BackendFactory {
    bool init() override;

    bool querySupport(BackendType type) override;

    std::string probe(BackendType type) override;

    BackendPtr createBackend(DeviceBase *device, BackendType type) override;

    static BackendFactory &getFactory();
};

#endif /* BACKENDS_AHI_H */

// Copyright (c) 2011, Cornell University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of HyperDex nor the names of its contributors may be
//       used to endorse or promote products derived from this software without
//       specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#define __STDC_LIMIT_MACROS

// HyperspaceHashing
#include <hyperspacehashing/mask.h>

// HyperDisk
#include "shard.h"
#include "shard_snapshot.h"

hyperdisk :: shard_snapshot :: shard_snapshot(uint32_t offset, shard* s)
    : m_shard(s)
    , m_limit(offset)
    , m_entry(0)
    , m_valid(true)
{
}

hyperdisk :: shard_snapshot :: shard_snapshot(const shard_snapshot& other)
    : m_shard(other.m_shard)
    , m_limit(other.m_limit)
    , m_entry(other.m_entry)
    , m_valid(other.m_valid)
{
}

hyperdisk :: shard_snapshot :: ~shard_snapshot() throw ()
{
}

bool
hyperdisk :: shard_snapshot :: valid()
{
    uint32_t offset = 0;
    uint32_t invalid = 0;

    while (m_entry < SEARCH_INDEX_ENTRIES)
    {
        offset = static_cast<uint32_t>(m_shard->m_search_log[m_entry * 2 + 1]);
        invalid = static_cast<uint32_t>(m_shard->m_search_log[m_entry * 2 + 1] >> 32);

        // If the m_valid flag is set; the offset is within the subsection of
        // data we may observe; and the data was never invalidated, or was
        // invalidated after we scanned it, then we may return true;
        if (m_valid && offset > 0 && offset < m_limit &&
                (invalid == 0 || invalid >= m_limit))
        {
            return true;
        }

        // If offset is 0, then we know that there are no more entries further
        // on.  Stop iterating.  If offset is >= m_limit, we know that the
        // operation (and all succeeding it) happened after the snapshot.
        if (offset == 0 || offset >= m_limit)
        {
            m_valid = false;
            break;
        }

        ++m_entry;
        m_valid = true;
    }

    return m_entry < SEARCH_INDEX_ENTRIES && m_valid;
}

void
hyperdisk :: shard_snapshot :: next()
{
    m_valid = false;
}

uint32_t
hyperdisk :: shard_snapshot :: primary_hash()
{
    return static_cast<uint32_t>(m_shard->m_search_log[m_entry * 2]);
}

uint32_t
hyperdisk :: shard_snapshot :: secondary_hash()
{
    return static_cast<uint32_t>(m_shard->m_search_log[m_entry * 2] >> 32);
}

hyperspacehashing::mask::coordinate
hyperdisk :: shard_snapshot :: coordinate()
{
    uint64_t hashes = m_shard->m_search_log[m_entry * 2];
    return hyperspacehashing::mask::coordinate(UINT32_MAX, static_cast<uint32_t>(hashes),
                                               UINT32_MAX, static_cast<uint32_t>(hashes >> 32));
}

uint64_t
hyperdisk :: shard_snapshot :: version()
{
    const uint32_t offset = m_shard->m_search_log[m_entry * 2 + 1] & 0xffffffffUL;

    if (!offset)
    {
        return uint64_t();
    }

    return m_shard->data_version(offset);
}

e::buffer
hyperdisk :: shard_snapshot :: key()
{
    const uint32_t offset = m_shard->m_search_log[m_entry * 2 + 1] & 0xffffffffUL;

    if (!offset)
    {
        return e::buffer();
    }

    e::buffer k;
    size_t key_size = m_shard->data_key_size(offset);
    m_shard->data_key(offset, key_size, &k);
    return k;
}

std::vector<e::buffer>
hyperdisk :: shard_snapshot :: value()
{
    const uint32_t offset = m_shard->m_search_log[m_entry * 2 + 1] & 0xffffffffUL;

    if (!offset)
    {
        return std::vector<e::buffer>();
    }

    std::vector<e::buffer> v;
    size_t key_size = m_shard->data_key_size(offset);
    m_shard->data_value(offset, key_size, &v);
    return v;
}

hyperdisk::shard_snapshot&
hyperdisk :: shard_snapshot :: operator = (const shard_snapshot& rhs)
{
    if (this != &rhs)
    {
        m_shard = rhs.m_shard;
        m_limit = rhs.m_limit;
        m_entry = rhs.m_entry;
        m_valid = rhs.m_valid;
    }

    return *this;
}

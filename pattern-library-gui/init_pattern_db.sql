CREATE TABLE IF NOT EXISTS patterns (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    modified_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS pattern_data (
    pattern_id INTEGER REFERENCES patterns(id),
    layer_number INTEGER NOT NULL,
    datatype_number INTEGER NOT NULL,
    geometry_data BYTEA NOT NULL,
    PRIMARY KEY (pattern_id, layer_number, datatype_number)
);

CREATE INDEX IF NOT EXISTS idx_pattern_name ON patterns(name);
CREATE INDEX IF NOT EXISTS idx_pattern_data_pattern_id ON pattern_data(pattern_id);

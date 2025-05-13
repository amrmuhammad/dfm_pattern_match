#!/bin/bash

# Database credentials
DB_NAME="pattern_library"
DB_USER="$USER"

echo "Creating database and user..."
sudo -u postgres psql << EOF
CREATE DATABASE $DB_NAME;
CREATE USER $DB_USER WITH PASSWORD '$DB_USER';
GRANT ALL PRIVILEGES ON DATABASE $DB_NAME TO $DB_USER;
ALTER USER $DB_USER WITH SUPERUSER;
EOF

echo "Setting up database schema..."
sudo -u postgres psql -d $DB_NAME << EOF
-- Grant schema privileges
GRANT ALL ON SCHEMA public TO $DB_USER;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO $DB_USER;

-- Create tables
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

-- Grant table privileges
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO $DB_USER;
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO $DB_USER;
EOF

echo "Database initialization complete!"

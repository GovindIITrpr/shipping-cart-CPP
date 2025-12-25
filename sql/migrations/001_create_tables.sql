-- Create cart_items table
DROP TABLE IF EXISTS cart_items CASCADE;

CREATE TABLE cart_items (
    id SERIAL PRIMARY KEY,
    item_id INT NOT NULL UNIQUE,
    name TEXT NOT NULL,
    price NUMERIC(12,2) NOT NULL,
    quantity INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create index on item_id for faster lookups
CREATE INDEX idx_item_id ON cart_items(item_id);

-- Insert sample data
INSERT INTO cart_items (item_id, name, price, quantity) VALUES
    (1, 'Laptop', 999.99, 1),
    (2, 'Mouse', 29.99, 2),
    (3, 'Keyboard', 79.99, 1);

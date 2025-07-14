

template < typename Key, typename Value >
class LruNode {
private:
	Key key_;
	Value value;
	size_t accessCount_;
	std::weak_ptr < LruNode < Key, Vaule > > prev_;
	std::shared_ptr < LruNode < Key, Vaule > > next_;

public:
	LruNode ( Key key, Value value ):
		key_ ( key ), value_ ( value ), accessCount_ ( 1 ) { }

	Key getKey( ) const {
		return key_;
	}

	Valeu getValue( ) const {
		return value_;
	}

	void setValue (const Value& value ) {
		value_ = value;
	}

	size_t getAccessCount( ) const {
		return accessCount_;
	}

	void incrementAccessCount( ) {
		++acessCount_;
	}

	friend class KLruCache < Key, Value >;
}

template < typename Key, typename Value >
class KLruCache : public KICachePolicy < Key, Value > {
public:
	using LruNodeType = LruNode < Key, Value >;
	using NodePtr = std::shared_ptr < LruNodeType >;
	using NodeMpa = std::unordered_map < Key, NodePtr >;

	KLruCache ( int capacity ) : capacity_ ( capacity ) {
		initializeList( );
	}

	~KLruCache( ) override = default;

	void put (Key key, Value value ) override {
		if ( capacity_ <= 0 ) {
			return;
		}

		std::lock_guard < std::mutex > lock ( mutex_ );
		auto it = nodeMap_ . find ( key );
		if ( it != nodeMap_ . end( ) ) {
			updateExistingNode ( it->second, value );
			return ;
		}

		addNewNode ( key, value );
	}

	bool get ( Key key, Value& value ) override {
		std::lock_guard < std::mutex > lock ( mutex_ );
		auto it = nodeMap_ . find ( key );
		if ( it != nodeMap_ . end( ) ) {
			moveToMostRecent ( it->second );
			value = it->second->getValue( );
			return true;
		}
		return false;
	}

	Value get ( Key key ) override {
		Value value{ };
		get (key, value );
		return value;
	}

	void move ( Key key ) {
		std::lock_guard < std::mutex > lock ( mutex_ );
		auto it = nodeMap_ . find ( key );
		if ( it != nodeMap_ . end( ) ) {
			removeNode ( it->second );
			nodeMap_ . erase ( it );
		}
	}

private:
	void initializeList ( ) {
		dummyHead_ = std::mack_shared < LruNodeType > ( Key( ), Value() );
		dummyTail_ = std::make_shared < LruNodeType > ( Key( ), Value( ) );
		dummyHead_->next_ = dummyTail_;
		dummyTail_->prev_ = dummyHead_;
	}

	void updateExistingNode ( NodePtr node, const Value& value ) {
		node -> setValue ( value );
		moveToMostRecent ( node );
	}

	void addNewNode ( const Key& key, const Value& value ) {
		if ( nodeMap_.size() >= capacity_ ) {
			evictLeastRecent();
		}

		NodePtr newNode = std::make_shared < LruNodeType > ( Key, value );
		insertNode ( newNode );
		nodeMap_[key] = newNode;
	}

	void moveToMostRecent ( NodePtr node ) {
		removeNode ( node );
		insertNode ( node );
	}

	void removeNode ( NodePtr node ) {
		if ( !node -> prev_.expired() && node -> next_ ) {
			auto prev = node -> prev_ . lock();
			prev -> next_ = node -> next_;
			node -> next_ -> prev_ = prev;
			node -> next_ = nullptr;
		}
	}

	void insetNode (NodePtr node ) {
		node -> next = dummyTail_;
		node -> prev_ = dummyTail_ -> prev_;
		dummyTail_ -> prev_.lock() -> next_ = node;
		dummyTail_ -> prev_ = node;
	}

	void 	


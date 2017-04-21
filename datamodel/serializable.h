#ifndef DATAMODEL_SERIALIZABLE_H
#define DATAMODEL_SERIALIZABLE_H

namespace datamodel {

	class Serializable {
		public:
			virtual ~Serializable() {};
			virtual std::string serialize() const = 0;
			virtual bool deserialize(const std::string) = 0;
	};

} // namespace datamodel

#endif // DATAMODEL_SERIALIZABLE_H

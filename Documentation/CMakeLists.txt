cmake_minimum_required(VERSION 3.18)

find_program(PANDOC_EXECUTABLE pandoc REQUIRED)

set(PANDOC_OPTS --toc --toc-depth=2
	--standalone
	--css=../style.css
	--template=template.html)

set(PANDOC_CHUNKEDOPTS ${PANDOC_OPTS}
	--write=chunkedhtml
	--embed-resources
	--split-level=1)

# Common
set(OUTPUT_HTML ${CMAKE_CURRENT_BINARY_DIR}/doc)
install(DIRECTORY ${OUTPUT_HTML}/ DESTINATION ${CMAKE_INSTALL_DOCDIR})

set(OUTPUT_CHUNKEDHTML ${CMAKE_CURRENT_BINARY_DIR}/chunkedhtml)
file(MAKE_DIRECTORY ${OUTPUT_CHUNKEDHTML})

# common
file(GLOB DOCDATA menu_*.png style.css)
file(COPY ${DOCDATA} DESTINATION ${OUTPUT_HTML})
file(COPY ${DOCDATA} DESTINATION ${OUTPUT_CHUNKEDHTML})

# German
set(OUTPUT_HTML_DE ${OUTPUT_HTML}/de)
set(OUTPUT_HTML_DE_INDEX ${OUTPUT_HTML_DE}/index.html)
set(OUTPUT_CHUNKEDHTML_DE ${OUTPUT_CHUNKEDHTML}/de)
set(OUTPUT_CHUNKEDHTML_DE_INDEX ${OUTPUT_CHUNKEDHTML_DE}/index.html)

file(GLOB DOCDATA_DE de/*.png)
file(COPY ${DOCDATA_DE} DESTINATION ${OUTPUT_HTML_DE})

file(GLOB DOC_DE de/*.md)
file(MAKE_DIRECTORY ${OUTPUT_HTML_DE})
add_custom_command(
       OUTPUT ${OUTPUT_HTML_DE_INDEX}
       COMMAND ${PANDOC_EXECUTABLE} ${PANDOC_OPTS}
               --metadata-file=de/metadata.yaml ${DOC_DE}
               --output=${OUTPUT_HTML_DE_INDEX}
       DEPENDS ${DOC_DE} ${OUTPUT_HTML_DE}
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
       COMMENT "Generating German HTML documentation"
       VERBATIM
)

file(MAKE_DIRECTORY ${OUTPUT_CHUNKEDHTML_DE})
add_custom_command(
	OUTPUT ${OUTPUT_CHUNKEDHTML_DE_INDEX}
	COMMAND ${PANDOC_EXECUTABLE} ${PANDOC_CHUNKEDOPTS}
		--resource-path=".:de:doc:doc/de"
		--metadata-file=de/metadata.yaml
		--output=${OUTPUT_CHUNKEDHTML_DE_INDEX} ${DOC_DE}
	DEPENDS ${DOC_DE} ${OUTPUT_CHUNKEDHTML_DE}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Generating German Chunked HTML documentation"
	VERBATIM
)

# English
set(OUTPUT_HTML_EN ${OUTPUT_HTML}/en)
set(OUTPUT_HTML_EN_INDEX ${OUTPUT_HTML_EN}/index.html)
set(OUTPUT_CHUNKEDHTML_EN ${OUTPUT_CHUNKEDHTML}/en)
set(OUTPUT_CHUNKEDHTML_EN_INDEX ${OUTPUT_CHUNKEDHTML_EN}/index.html)

file(GLOB DOCDATA_EN en/*.png)
file(COPY ${DOCDATA_EN} DESTINATION ${OUTPUT_HTML_EN})

file(GLOB DOC_EN en/*.md)
file(MAKE_DIRECTORY ${OUTPUT_HTML_EN})
add_custom_command(
       OUTPUT ${OUTPUT_HTML_EN_INDEX}
       COMMAND ${PANDOC_EXECUTABLE} ${PANDOC_OPTS}
               --metadata-file=en/metadata.yaml ${DOC_EN}
               --output=${OUTPUT_HTML_EN_INDEX}
       DEPENDS ${DOC_EN} ${OUTPUT_HTML_EN}
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
       COMMENT "Generating English HTML documentation"
       VERBATIM
)

file(MAKE_DIRECTORY ${OUTPUT_CHUNKEDHTML_EN})
add_custom_command(
	OUTPUT ${OUTPUT_CHUNKEDHTML_EN_INDEX}
	COMMAND ${PANDOC_EXECUTABLE} ${PANDOC_CHUNKEDOPTS}
		--resource-path=".:en:doc:doc/en"
		--metadata-file=en/metadata.yaml
		--output=${OUTPUT_CHUNKEDHTML_EN_INDEX} ${DOC_EN}
	DEPENDS ${DOC_EN} ${OUTPUT_CHUNKEDHTML_EN}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Generating German Chunked HTML documentation"
	VERBATIM
)

# Spanish
set(OUTPUT_HTML_ES ${OUTPUT_HTML}/es)
set(OUTPUT_HTML_ES_INDEX ${OUTPUT_HTML_ES}/index.html)
set(OUTPUT_CHUNKEDHTML_ES ${OUTPUT_CHUNKEDHTML}/es)
set(OUTPUT_CHUNKEDHTML_ES_INDEX ${OUTPUT_CHUNKEDHTML_ES}/index.html)

file(GLOB DOCDATA_ES es/*.png)
file(COPY ${DOCDATA_ES} DESTINATION ${OUTPUT_HTML_ES})

file(GLOB DOC_ES es/*.md)
file(MAKE_DIRECTORY ${OUTPUT_HTML_ES})
add_custom_command(
       OUTPUT ${OUTPUT_HTML_ES_INDEX}
       COMMAND ${PANDOC_EXECUTABLE} ${PANDOC_OPTS}
               --metadata-file=es/metadata.yaml ${DOC_ES}
               --output=${OUTPUT_HTML_ES_INDEX}
       DEPENDS ${DOC_ES} ${OUTPUT_HTML_ES}
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
       COMMENT "Generating English HTML documentation"
       VERBATIM
)

file(MAKE_DIRECTORY ${OUTPUT_CHUNKEDHTML_ES})
add_custom_command(
	OUTPUT ${OUTPUT_CHUNKEDHTML_ES_INDEX}
	COMMAND ${PANDOC_EXECUTABLE} ${PANDOC_CHUNKEDOPTS}
		--resource-path=".:es:en:doc:doc/es"
		--metadata-file=es/metadata.yaml
		--output=${OUTPUT_CHUNKEDHTML_ES_INDEX} ${DOC_ES}
	DEPENDS ${DOC_ES} ${OUTPUT_CHUNKEDHTML_ES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Generating German Chunked HTML documentation"
	VERBATIM
)

# build targets
add_custom_target(doc ALL
	DEPENDS ${OUTPUT_HTML_DE_INDEX} ${OUTPUT_HTML_EN_INDEX} ${OUTPUT_HTML_ES_INDEX}
)

add_custom_target(chunked-doc ALL
	DEPENDS ${OUTPUT_CHUNKEDHTML_DE_INDEX} ${OUTPUT_CHUNKEDHTML_EN_INDEX} ${OUTPUT_CHUNKEDHTML_ES_INDEX}
)

# Web
add_custom_target(doc-for-web
	DEPENDS doc
)

add_custom_command(TARGET doc-for-web POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_CURRENT_SOURCE_DIR}/de/documentation-de
	${PROJECT_BINARY_DIR}/doc/de/documentation-de
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_CURRENT_SOURCE_DIR}/en/documentation-en
	${PROJECT_BINARY_DIR}/doc/en/documentation-en
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_CURRENT_SOURCE_DIR}/es/documentation-es
	${PROJECT_BINARY_DIR}/doc/es/documentation-es
)

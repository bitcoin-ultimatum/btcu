
# MacOS-only options:
# ---------------------------------------
# SIGN_KEYCHAIN      - keychain where cert is located (optional: defaults to login keychain, if not provided)
# SIGN_ENTITLEMENTS  - path to entitlements file (optional: default is no entitlements file)
#
# For details on using the entitlements file to enable app sandboxing, see here:
#   https://developer.apple.com/library/content/documentation/Miscellaneous/Reference/EntitlementKeyReference
#
# Note: Using an entitlements file will enable hardended run time. Hardended entitlements may be necessary.
#
# On MacOS, the name passed to SIGN_CERT_NAME should match all or part of the "Common Name" field in the certificate.
# Examples of certificate names:
#    "iPhone Distribution: BTCU Core, LLC."
#    "Mac Developer: John Smith" (Used for testing and development on personal machine)
#    "3rd Party Mac Developer Application: BTCU Core, LLC." (ONLY USE FOR APP STORE)
#    "Developer ID Application: BTCU Core, LLC." (USE FOR DIRECT DISTRIBUTION OUTSIDE THE APP STORE)
#
# If there is only one certificate loaded on your machine for each type of platform / deployment scenario, you
# can get away with specifying only the generic part (up to the colon). For example, if only one Mac distribution
# certificate is installed on the machine, setting SIGN_CERT_NAME to "Mac Developer Application" will find the
# proper key.

# Helper for code_sign_files_macos
function(code_sign_get_cmd_args_macos out_args)
	set(code_sign_ards "")

	if (SIGN_ENTITLEMENTS)
		# Enable hardened runtime
		set(code_sign_ards "${code_sign_ards} --options=runtime  --entitlements '${SIGN_ENTITLEMENTS}'")
	endif ()
   
	set(code_sign_ards "${code_sign_ards} --sign '${SIGN_CERT_NAME}' --timestamp --signature-size=12000")

	if (SIGN_KEYCHAIN)
		set(code_sign_ards "${code_sign_ards} --keychain '${SIGN_KEYCHAIN}'")
	endif ()

	set(${out_args} "${code_sign_ards}" PARENT_SCOPE)
endfunction()

# Helper for code_sign_files_macos
function(code_sign_get_identity_macos out_identity)
	# Get list of valid codesigning identities from system.
	execute_process(COMMAND security find-identity -v -p codesigning
		RESULT_VARIABLE res
		OUTPUT_VARIABLE lines
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	set(result "")

	if (DEFINED SIGN_CERT_NAME)
		set(result "${SIGN_CERT_NAME}")
	else()
		if (res EQUAL 0 AND lines)
			# Split string into list of lines.
			string(REGEX REPLACE ";" "\\\\;" lines "${lines}")
			string(REGEX REPLACE "\n" ";" lines "${lines}")
			# Parse signing cert identity from each line
			foreach(line ${lines})
				# Ex: 1) EE484E4BB4CE4779E5BF2AE3342636A035CC359 "iPhone Developer: Stephen Sorley (6JWJW49L4N)"
				if (line MATCHES "[0-9]+\\)[ \t]+[0-9a-fA-F]+[ \t]+\"(.+) \\([^ \t]+\\)\"")
					set(result "${CMAKE_MATCH_1}")
					break()
				endif ()
			endforeach()
		endif ()
	endif ()	
	set(${out_identity} "${result}" PARENT_SCOPE)
endfunction()
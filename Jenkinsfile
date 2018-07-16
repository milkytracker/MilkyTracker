def notify(status){
	emailext (
		body: '$DEFAULT_CONTENT', 
		recipientProviders: [
			[$class: 'CulpritsRecipientProvider'],
			[$class: 'DevelopersRecipientProvider'],
			[$class: 'RequesterRecipientProvider']
		], 
		replyTo: '$DEFAULT_REPLYTO', 
		subject: '$DEFAULT_SUBJECT',
		to: '$DEFAULT_RECIPIENTS'
	)
}

def buildStep(config, ext) {
	sh "make clean config=$config"
	sh "make config=$config -j8"
	if (!env.CHANGE_ID) {
		sh "mv bin/milkytracker.$ext publishing/deploy/MilkyTracker/"
		//sh "cp publishing/amiga-spec/MilkyTracker.info publishing/deploy/MilkyTracker/MilkyTracker.$ext.info"
	}
}

env.PATH = env.FORCEDPATH

node {
	try{
		stage('Checkout and pull') {
			properties([pipelineTriggers([githubPush()])])
			if (env.CHANGE_ID) {
				echo 'Trying to build pull request'
			}

			echo "$BRANCH_NAME"
			echo "${env.BRANCH_NAME}"

			checkout scm
		}
	if ($TAG_NAME) {
		echo "$TAG_NAME"
	} else {
		stage('Clean workspace') {
			sh "./clean"
		}

		stage('Generate makefiles') {
			sh "./build_gmake"
		}

		if (!env.CHANGE_ID) {
			stage('Generate publishing directories') {
				sh "rm -rfv publishing/deploy/*"
				sh "mkdir -p publishing/deploy/MilkyTracker"
			}
		}

		stage('Build AmigaOS 3.x version') {
			buildStep('release_m68k-amigaos', '68k')
		}

		stage('Build AmigaOS 4.x version') {
			buildStep('release_ppc-amigaos', 'os4')
		}

		stage('Build MorphOS 3.x version') {
			buildStep('release_ppc-morphos', 'mos')
		}

		stage('Build AROS x86 ABI-v1 version') {
			buildStep('release_i386-aros', 'aros-abiv1')
		}

		stage('Deploying to stage') {
			sh "ls -l publishing/deploy/MilkyTracker"
			
			
			
		}
	}
	} catch(err) {
		currentBuild.result = 'FAILURE'
		notify('Build failed')
	}
}

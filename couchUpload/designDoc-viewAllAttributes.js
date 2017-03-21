function(doc) {
    emit(null, {
        'userID': doc.userID,
        'studyID': doc.studyID,
        'trialID': doc.trialID,
        'skillLevel': doc.skillLevel,
        'date': doc.date,
        'sceneName': doc.sceneName,
        'id': doc._id
    });
}
const project = new Project('Audio-Test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addKongDir('shaders');
project.setDebugDir('deployment');

project.flatten();

resolve(project);
